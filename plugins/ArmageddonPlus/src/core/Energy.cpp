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
    g_enabled = config.GetBool("Energy", "Enabled", true);
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

void Draw(float rightX, float topY) {
    if (!g_enabled)
        return;

    float width = SCREEN_COORD(150.0f), height = SCREEN_COORD(9.0f), border = SCREEN_COORD(2.0f);
    float left = rightX - width;
    float fraction = g_max > 0.0f ? g_current / g_max : 0.0f;

    // fill color: blue, orange when low, flashing red after a failed spend
    CRGBA fill(70, 150, 255, 230);
    if (fraction < 0.25f)
        fill = CRGBA(255, 150, 40, 230);
    if (NowMs() < g_failFlashUntil && (NowMs() / 100) % 2 == 0)
        fill = CRGBA(255, 40, 40, 255);

    CSprite2d::DrawRect(CRect(left - border, topY - border, rightX + border, topY + height + border), CRGBA(0, 0, 0, 200));
    CSprite2d::DrawRect(CRect(left, topY, rightX, topY + height), CRGBA(40, 40, 60, 200));
    if (fraction > 0.0f)
        CSprite2d::DrawRect(CRect(left, topY, left + width * fraction, topY + height), fill);

    char text[16];
    std::snprintf(text, sizeof(text), "%d", (int)std::ceil(g_current));
    CFont::SetOrientation(ALIGN_RIGHT);
    CFont::SetRightJustifyWrap(0.0f);
    CFont::SetFontStyle(FONT_SUBTITLES);
    CFont::SetProportional(true);
    CFont::SetBackground(false, false);
    CFont::SetScale(SCREEN_MULTIPLIER(0.3f), SCREEN_MULTIPLIER(0.7f));
    CFont::SetDropShadowPosition(1);
    CFont::SetDropColor(CRGBA(0, 0, 0, 255));
    CFont::SetColor(CRGBA(255, 255, 255, 255));
    CFont::PrintString(left - SCREEN_COORD(6.0f), topY - SCREEN_COORD(3.0f), text);
}

} // namespace Energy
