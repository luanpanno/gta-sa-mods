#include "core/Sound.h"

#include "core/Config.h"
#include "core/Log.h"

#include "plugin.h"

#include <windows.h>
#include <mmsystem.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

#pragma comment(lib, "winmm.lib") // PlaySound fallback

namespace {

const int kRate = 22050; // samples per second (mono, 16-bit)
const float kPi = 3.14159265f;

// ---- BASS, loaded at runtime from the game folder (shipped with CLEO) ----------------------
using HSAMPLE = DWORD;
using HCHANNEL = DWORD;
using QWORD = unsigned long long;
using FnInit = BOOL(WINAPI*)(int device, DWORD freq, DWORD flags, HWND win, const void* clsid);
using FnGetDevice = DWORD(WINAPI*)();
using FnSampleLoad = HSAMPLE(WINAPI*)(BOOL mem, const void* file, QWORD offset, DWORD length, DWORD max, DWORD flags);
using FnSampleGetChannel = HCHANNEL(WINAPI*)(HSAMPLE handle, DWORD flags);
using FnChannelSetAttribute = BOOL(WINAPI*)(DWORD handle, DWORD attrib, float value);
using FnChannelPlay = BOOL(WINAPI*)(DWORD handle, BOOL restart);
const DWORD BASS_ATTRIB_VOL = 2;
const DWORD BASS_SAMPLE_OVER_POS = 0x40000; // when all 4 voices are busy, reuse the oldest

struct Bass {
    bool tried = false, ok = false;
    FnInit Init = nullptr;
    FnGetDevice GetDevice = nullptr;
    FnSampleLoad SampleLoad = nullptr;
    FnSampleGetChannel SampleGetChannel = nullptr;
    FnChannelSetAttribute ChannelSetAttribute = nullptr;
    FnChannelPlay ChannelPlay = nullptr;
} g_bass;

bool g_enabled = true;
float g_volume = 0.8f;
std::vector<uint8_t> g_wav[Sound::Count]; // complete .wav files in memory
HSAMPLE g_samples[Sound::Count] = {};

const char* kNames[Sound::Count] = {"whoosh", "whoosh_in", "thunder", "zap", "slowdown", "speedup"};

// ---- tiny synthesizer --------------------------------------------------------------------
struct Noise { // deterministic white noise, -1..1
    uint32_t state = 0x12345678;
    float Next() {
        state = state * 1664525u + 1013904223u;
        return ((state >> 8) & 0xFFFF) / 32768.0f - 1.0f;
    }
};

struct OnePole { // one-pole low-pass filter; cutoff in Hz
    float y = 0.0f;
    float Process(float x, float cutoff) {
        float a = 1.0f - std::exp(-2.0f * kPi * cutoff / kRate);
        y += a * (x - y);
        return y;
    }
};

std::vector<float> SynthWhoosh(float seconds) {
    std::vector<float> out((size_t)(seconds * kRate));
    Noise noise;
    OnePole low, high;
    for (size_t i = 0; i < out.size(); ++i) {
        float t = (float)i / out.size();                    // 0..1
        float env = std::pow(std::sin(kPi * t), 1.5f);      // swell in and out
        float cutoff = 300.0f + 2200.0f * std::sin(kPi * t); // brighter in the middle
        float n = noise.Next();
        float band = low.Process(n, cutoff) - high.Process(n, 150.0f); // band-pass
        out[i] = band * env;
    }
    return out;
}

std::vector<float> SynthThunder(float seconds) {
    std::vector<float> out((size_t)(seconds * kRate));
    Noise noise;
    OnePole crackFilter, rumbleFilter, rumbleFilter2;
    for (size_t i = 0; i < out.size(); ++i) {
        float t = (float)i / kRate;
        float n = noise.Next();
        // sharp crack: bright noise, gone in ~0.15 s
        float crack = (n - crackFilter.Process(n, 800.0f)) * std::exp(-t * 25.0f) * 1.2f;
        // rumble: dark noise, slow decay, with a second roll at ~0.5 s
        float rumble = rumbleFilter2.Process(rumbleFilter.Process(n, 180.0f), 120.0f) * 6.0f;
        float rollEnv = std::exp(-t * 1.6f) + 0.6f * std::exp(-std::fabs(t - 0.5f) * 4.0f);
        float attack = std::min(1.0f, t / 0.03f);
        out[i] = crack + rumble * rollEnv * attack;
    }
    return out;
}

std::vector<float> SynthZap(float seconds) {
    std::vector<float> out((size_t)(seconds * kRate));
    Noise noise;
    float phase = 0.0f;
    for (size_t i = 0; i < out.size(); ++i) {
        float t = (float)i / kRate;
        float freq = 1400.0f * std::exp(-t * 7.0f) + 180.0f; // falling pitch
        phase += 2.0f * kPi * freq / kRate;
        float tone = std::sin(phase) + 0.35f * std::sin(phase * 3.0f) + 0.15f * noise.Next();
        out[i] = tone * std::exp(-t * 9.0f);
    }
    return out;
}

std::vector<float> SynthSweep(float seconds, float fromHz, float toHz) {
    std::vector<float> out((size_t)(seconds * kRate));
    Noise noise;
    OnePole low;
    float phase = 0.0f;
    for (size_t i = 0; i < out.size(); ++i) {
        float t = (float)i / out.size();
        float freq = fromHz * std::pow(toHz / fromHz, t); // exponential glide
        phase += 2.0f * kPi * freq / kRate;
        float tone = std::sin(phase) + 0.5f * std::sin(phase * 0.5f) + 0.6f * low.Process(noise.Next(), 300.0f);
        float env = std::sin(kPi * t);
        out[i] = tone * env;
    }
    return out;
}

std::vector<uint8_t> ToWav(std::vector<float> samples) {
    float peak = 0.0001f;
    for (float s : samples)
        peak = std::max(peak, std::fabs(s));
    uint32_t dataBytes = (uint32_t)samples.size() * 2;
    std::vector<uint8_t> wav(44 + dataBytes);
    auto put32 = [&](size_t at, uint32_t v) { std::memcpy(&wav[at], &v, 4); };
    auto put16 = [&](size_t at, uint16_t v) { std::memcpy(&wav[at], &v, 2); };
    std::memcpy(&wav[0], "RIFF", 4);
    put32(4, 36 + dataBytes);
    std::memcpy(&wav[8], "WAVEfmt ", 8);
    put32(16, 16);         // fmt chunk size
    put16(20, 1);          // PCM
    put16(22, 1);          // mono
    put32(24, kRate);
    put32(28, kRate * 2);  // bytes per second
    put16(32, 2);          // block align
    put16(34, 16);         // bits per sample
    std::memcpy(&wav[36], "data", 4);
    put32(40, dataBytes);
    for (size_t i = 0; i < samples.size(); ++i) {
        int16_t v = (int16_t)(samples[i] / peak * 0.9f * 32767.0f); // normalize to 90%
        std::memcpy(&wav[44 + i * 2], &v, 2);
    }
    return wav;
}

bool LoadCustom(const char* name, std::vector<uint8_t>& out) {
    std::string path = std::string(plugin::paths::GetPluginDirPathA()) + "sounds\\" + name + ".wav";
    std::ifstream file(path, std::ios::binary);
    if (!file)
        return false;
    out.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    Log::Info("Sound: using custom %s", path.c_str());
    return !out.empty();
}

void LoadBass() {
    g_bass.tried = true;
    HMODULE dll = GetModuleHandleA("bass.dll");
    if (!dll)
        dll = LoadLibraryA("bass.dll"); // game folder (CLEO ships it)
    if (!dll) {
        Log::Info("Sound: bass.dll not found, using PlaySound");
        return;
    }
    g_bass.Init = (FnInit)GetProcAddress(dll, "BASS_Init");
    g_bass.GetDevice = (FnGetDevice)GetProcAddress(dll, "BASS_GetDevice");
    g_bass.SampleLoad = (FnSampleLoad)GetProcAddress(dll, "BASS_SampleLoad");
    g_bass.SampleGetChannel = (FnSampleGetChannel)GetProcAddress(dll, "BASS_SampleGetChannel");
    g_bass.ChannelSetAttribute = (FnChannelSetAttribute)GetProcAddress(dll, "BASS_ChannelSetAttribute");
    g_bass.ChannelPlay = (FnChannelPlay)GetProcAddress(dll, "BASS_ChannelPlay");
    if (!g_bass.Init || !g_bass.GetDevice || !g_bass.SampleLoad || !g_bass.SampleGetChannel ||
        !g_bass.ChannelSetAttribute || !g_bass.ChannelPlay)
        return;
    // CLEO's audio plugin normally initializes BASS already; if not, do it ourselves
    if (g_bass.GetDevice() == (DWORD)-1 && !g_bass.Init(-1, 44100, 0, nullptr, nullptr)) {
        Log::Info("Sound: BASS_Init failed, using PlaySound");
        return;
    }
    for (int i = 0; i < Sound::Count; ++i)
        g_samples[i] = g_bass.SampleLoad(TRUE, g_wav[i].data(), 0, (DWORD)g_wav[i].size(), 4, BASS_SAMPLE_OVER_POS);
    g_bass.ok = true;
    Log::Info("Sound: using BASS");
}

} // namespace

namespace Sound {

void LoadConfig(const Config& config) {
    g_enabled = config.GetBool("Effects", "Sounds", true);
    g_volume = config.GetFloat("Effects", "Volume", g_volume);

    std::vector<float> whoosh = SynthWhoosh(0.7f);
    std::vector<float> whooshIn(whoosh.rbegin(), whoosh.rend()); // reversed: sucks in
    std::vector<float> built[Count] = {whoosh, whooshIn, SynthThunder(2.2f), SynthZap(0.3f),
                                       SynthSweep(0.9f, 240.0f, 55.0f), SynthSweep(0.7f, 55.0f, 240.0f)};
    for (int i = 0; i < Count; ++i)
        if (!LoadCustom(kNames[i], g_wav[i]))
            g_wav[i] = ToWav(built[i]);
}

void Play(Id id, float volume) {
    if (!g_enabled || id < 0 || id >= Count || g_wav[id].empty())
        return;
    if (!g_bass.tried)
        LoadBass(); // lazily: by the first power use, the game (and CLEO) are fully up
    if (g_bass.ok && g_samples[id]) {
        HCHANNEL channel = g_bass.SampleGetChannel(g_samples[id], 0);
        if (channel) {
            g_bass.ChannelSetAttribute(channel, BASS_ATTRIB_VOL, std::clamp(volume * g_volume, 0.0f, 1.0f));
            g_bass.ChannelPlay(channel, FALSE);
        }
        return;
    }
    PlaySoundA((LPCSTR)g_wav[id].data(), nullptr, SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
}

} // namespace Sound
