#include "core/Energy.h"

#include "core/Config.h"

#include "plugin.h"
#include "CAudioEngine.h"
#include "CFont.h"
#include "CMessages.h"
#include "CRect.h"
#include "CSprite2d.h"

#include <windows.h>

#include <cmath>
#include <cstdio>

using namespace plugin;

namespace {

bool g_enabled = true;
float g_max = 100.0f;
float g_current = 100.0f;
float g_regen = 8.0f;        // per second
float g_regenDelay = 1.5f;   // seconds without spending before regen starts
float g_minToStart = 5.0f;   // need at least this much to switch a power on

float g_sinceSpent = 0.0f;   // real seconds since energy was last spent
unsigned long long g_failFlashUntil = 0; // bar flashes red until this time (ms)

unsigned long long NowMs() {
    return GetTickCount64();
}

void Fail() {
    AudioEngine.ReportFrontendAudioEvent(AE_FRONTEND_ERROR, 0.0f, 1.0f);
    g_failFlashUntil = NowMs() + 600;
}

} // namespace

namespace Energy {

void LoadConfig(const Config& config) {
    g_enabled = config.GetBool("Energy", "Enabled", false); // off by default: infinite energy
    g_max = config.GetFloat("Energy", "Max", g_max);
    g_regen = config.GetFloat("Energy", "Regen", g_regen);
    g_regenDelay = config.GetFloat("Energy", "RegenDelay", g_regenDelay);
    g_minToStart = config.GetFloat("Energy", "MinToStart", g_minToStart);
    g_current = g_max;
}

bool IsEnabled() {
    return g_enabled;
}

void SetEnabled(bool enabled) {
    g_enabled = enabled;
    if (enabled)
        g_current = g_max; // start full when switched on
}

float Current() {
    return g_current;
}

float Max() {
    return g_max;
}

void Update(float realDt) {
    g_sinceSpent += realDt;
    if (g_sinceSpent >= g_regenDelay && g_current < g_max) {
        g_current += g_regen * realDt;
        if (g_current > g_max)
            g_current = g_max;
    }
}

bool TrySpend(float amount) {
    if (!g_enabled || amount <= 0.0f)
        return true;
    if (g_current < amount) {
        Fail();
        CMessages::AddMessageJumpQ("~r~Not enough energy", 1200, 0);
        return false;
    }
    g_current -= amount;
    g_sinceSpent = 0.0f;
    return true;
}

bool Drain(float amount) {
    if (!g_enabled || amount <= 0.0f)
        return true;
    g_sinceSpent = 0.0f;
    g_current -= amount;
    if (g_current <= 0.0f) {
        g_current = 0.0f;
        return false;
    }
    return true;
}

bool CanStart() {
    if (!g_enabled || g_current >= g_minToStart)
        return true;
    Fail();
    CMessages::AddMessageJumpQ("~r~Not enough energy", 1200, 0);
    return false;
}

bool IsFailFlashing() {
    return NowMs() < g_failFlashUntil && (NowMs() / 100) % 2 == 0;
}

} // namespace Energy
