#include "core/PowerManager.h"

#include "core/Config.h"
#include "core/Input.h"
#include "core/Log.h"

#include "plugin.h"
#include "CFont.h"
#include "CPlayerPed.h"
#include "CTimer.h"

#include <cstdio>
#include <cstring>

using namespace plugin;

void PowerManager::Register(std::unique_ptr<Power> power) {
    m_powers.push_back(std::move(power));
}

void PowerManager::LoadConfig(const Config& config) {
    m_overlayEnabled = config.GetBool("Debug", "Overlay", true);
    m_overlayKey = config.GetKey("Debug", "OverlayKey", 0);

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
    if (active && power.ExclusiveGroup()) {
        for (auto& other : m_powers)
            if (other.get() != &power && other->m_active && other->ExclusiveGroup() &&
                std::strcmp(other->ExclusiveGroup(), power.ExclusiveGroup()) == 0)
                SetActive(*other, player, false);
    }
    power.m_active = active;
    power.m_level = active ? 1 : 0;
    if (active) {
        power.OnActivate(player);
        power.OnLevelChanged(player, 1);
    } else {
        power.OnDeactivate(player);
    }
    Log::Info("%s %s", power.Id(), active ? "ON" : "OFF");
}

void PowerManager::StepLevel(Power& power, CPlayerPed* player) {
    if (!power.m_active) {
        SetActive(power, player, true); // off -> level 1
    } else if (power.m_level < power.MaxLevel()) {
        power.m_level++;
        power.OnLevelChanged(player, power.m_level);
        Log::Info("%s level %d", power.Id(), power.m_level);
    } else {
        SetActive(power, player, false); // max level -> off
    }
}

void PowerManager::SetLevel(Power& power, CPlayerPed* player, int level) {
    if (level < 0)
        level = 0;
    if (level > power.MaxLevel())
        level = power.MaxLevel();
    if (level == 0) {
        SetActive(power, player, false);
        return;
    }
    SetActive(power, player, true); // no-op if already on (sets level 1 if it was off)
    if (power.m_level != level) {
        power.m_level = level;
        power.OnLevelChanged(player, level);
        Log::Info("%s level %d", power.Id(), level);
    }
}

void PowerManager::Fire(Power& power, CPlayerPed* player) {
    power.OnActivate(player);
    Log::Info("%s fired", power.Id());
}

void PowerManager::DeactivateAll(CPlayerPed* player) {
    for (auto& power : m_powers)
        SetActive(*power, player, false);
}

void PowerManager::Update() {
    if (m_hotkeysEnabled && Input::JustPressed(m_overlayKey))
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
        // hotkeys are paused while the menu is open (it uses the keyboard), but powers keep running
        int key = m_hotkeysEnabled ? power->m_key : 0;
        switch (power->GetMode()) {
        case Power::Mode::Toggle:
            if (Input::JustPressed(key))
                StepLevel(*power, player);
            break;
        case Power::Mode::Hold:
            if (m_hotkeysEnabled)
                SetActive(*power, player, Input::IsDown(key));
            break;
        case Power::Mode::OneShot:
            if (Input::JustPressed(key))
                Fire(*power, player);
            break;
        }
        if (power->m_active)
            power->Update(player, dt);
        power->UpdateAlways(player, dt);
    }
}

void PowerManager::DrawPowers() {
    for (auto& power : m_powers)
        if (power->m_active)
            power->Draw();
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

        char key[24] = "";
        if (power->m_key) // only powers with an optional hotkey (Key= in the ini) show it
            std::snprintf(key, sizeof(key), "[%s] ", Config::KeyName(power->m_key));
        if (power->MaxLevel() > 1 && power->m_active)
            std::snprintf(line, sizeof(line), "%s%s %d/%d", key, power->Name(), power->m_level, power->MaxLevel());
        else
            std::snprintf(line, sizeof(line), "%s%s", key, power->Name());
        CFont::PrintString(SCREEN_COORD_RIGHT(20.0f), y, line);
        y += SCREEN_COORD(16.0f);
    }
}
