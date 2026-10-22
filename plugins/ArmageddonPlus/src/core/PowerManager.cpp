#include "core/PowerManager.h"

#include "core/Config.h"
#include "core/Input.h"
#include "core/Log.h"

#include "plugin.h"
#include "CFont.h"
#include "CPlayerPed.h"
#include "CTimer.h"

#include <cstdio>

using namespace plugin;

void PowerManager::Register(std::unique_ptr<Power> power) {
    m_powers.push_back(std::move(power));
}

void PowerManager::LoadConfig(const Config& config) {
    m_overlayEnabled = config.GetBool("Debug", "Overlay", true);
    m_overlayKey = config.GetKey("Debug", "OverlayKey", VK_F12);

    char section[64];
    for (auto& power : m_powers) {
        std::snprintf(section, sizeof(section), "Power.%s", power->Id());
        power->m_key = config.GetKey(section, "Key", power->DefaultKey());
        power->LoadConfig(config, section);
        Log::Info("Power %-16s key=%s", power->Id(), Config::KeyName(power->m_key));
    }
}

void PowerManager::SetActive(Power& power, CPlayerPed* player, bool active) {
    if (power.m_active == active)
        return;
    power.m_active = active;
    if (active)
        power.OnActivate(player);
    else
        power.OnDeactivate(player);
    Log::Info("%s %s", power.Id(), active ? "ON" : "OFF");
}

void PowerManager::DeactivateAll(CPlayerPed* player) {
    for (auto& power : m_powers)
        SetActive(*power, player, false);
}

void PowerManager::Update() {
    if (Input::JustPressed(m_overlayKey))
        m_overlayEnabled = !m_overlayEnabled;

    CPlayerPed* player = FindPlayerPed();
    if (!player)
        return;

    // dead (or being busted): drop every power so nothing keeps running on the respawn
    if (player->m_fHealth <= 0.0f) {
        DeactivateAll(player);
        return;
    }

    // CTimer::ms_fTimeStep is in "1/50th of a second" units; convert to seconds
    float dt = CTimer::ms_fTimeStep / 50.0f;

    for (auto& power : m_powers) {
        int key = power->m_key;
        switch (power->GetMode()) {
        case Power::Mode::Toggle:
            if (Input::JustPressed(key))
                SetActive(*power, player, !power->m_active);
            break;
        case Power::Mode::Hold:
            SetActive(*power, player, Input::IsDown(key));
            break;
        case Power::Mode::OneShot:
            if (Input::JustPressed(key)) {
                power->OnActivate(player);
                Log::Info("%s fired", power->Id());
            }
            break;
        }
        if (power->m_active)
            power->Update(player, dt);
    }
}

void PowerManager::DrawOverlay() {
    if (!m_overlayEnabled)
        return;

    float y = SCREEN_COORD_TOP(32.0f);
    char line[96];
    for (auto& power : m_powers) {
        CFont::SetOrientation(ALIGN_RIGHT);
        CFont::SetRightJustifyWrap(0.0f);
        CFont::SetFontStyle(FONT_SUBTITLES);
        CFont::SetProportional(true);
        CFont::SetBackground(false, false);
        CFont::SetScale(SCREEN_MULTIPLIER(0.35f), SCREEN_MULTIPLIER(0.8f));
        CFont::SetDropShadowPosition(1);
        CFont::SetDropColor(CRGBA(0, 0, 0, 255));
        CFont::SetColor(power->m_active ? CRGBA(120, 255, 120, 255) : CRGBA(200, 200, 200, 200));

        std::snprintf(line, sizeof(line), "[%s] %s", Config::KeyName(power->m_key), power->Name());
        CFont::PrintString(SCREEN_COORD_RIGHT(20.0f), y, line);
        y += SCREEN_COORD(16.0f);
    }
}
