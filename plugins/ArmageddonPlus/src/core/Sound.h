#pragma once

class Config;

// Phase 4.3: power sound effects.
//
// The game has no "thunder" / "whoosh" sounds we can trigger, so they're SYNTHESIZED at startup
// (noise + filters + envelopes, see Sound.cpp) into in-memory WAVs, and played through BASS:
// the audio library CLEO already ships (bass.dll in the game folder), loaded at runtime.
// Without BASS, it falls back to Windows' PlaySound (one sound at a time).
//
// Custom sounds: put e.g. "thunder.wav" next to the .asi (modloader\ArmageddonPlus\sounds\) and it
// replaces the synthesized one.
namespace Sound {

enum Id {
    Whoosh,     // flight take-off, force push, throws
    WhooshIn,   // force pull (reversed whoosh)
    Thunder,    // lightning strike
    Zap,        // telekinesis grab
    SlowDown,   // bullet time on
    SpeedUp,    // bullet time off
    Count
};

void LoadConfig(const Config& config); // also builds the sounds
void Play(Id id, float volume = 1.0f);  // 0..1, times [Effects] Volume

} // namespace Sound
