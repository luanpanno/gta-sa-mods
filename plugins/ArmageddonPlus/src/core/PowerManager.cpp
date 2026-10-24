#include "core/PowerManager.h"

#include "core/Config.h"
#include "core/Energy.h"
#include "core/Input.h"
#include "core/Log.h"

#include "plugin.h"
#include "CFont.h"
#include "CMessages.h"
#include "CPlayerPed.h"
#include "CTimer.h"

#include <cstdio>
#include <cstring>

using namespace plugin;

bool PowerManager::AnyActive(const char* category) const {
    for (auto& power : m_powers)
        if (power->m_active && std::strcmp(power->Category(), category) == 0)
            return true;
    return false;
}

void PowerManager::Register(std::unique_ptr<Power> power) {
    m_powers.push_back(std::move(power));
}

void PowerManager::LoadConfig(const Config& config) {
    m_overlayEnabled = config.GetBool("Debug", "Overlay", true);
    m_overlayKey = config.GetKey("Debug", "OverlayKey", 0);

    Energy::LoadConfig(config);

    char section[64], key[16];
    for (auto& power : m_powers) {
        std::snprintf(section, sizeof(section), "Power.%s", power->Id());
        power->m_key = config.GetKey(section, "Key", power->DefaultKey());
        // energy: "Drain"/"Cost" = same for every level, "Drain2"/"Cost2"... override per level
        float drain = config.GetFloat(section, "Drain", 0.0f);
        float cost = config.GetFloat(section, "Cost", 0.0f);
        for (int i = 0; i < Power::kMaxLevels; ++i) {
            std::snprintf(key, sizeof(key), "Drain%d", i + 1);
            power->m_drain[i] = config.GetFloat(section, key, drain);
            std::snprintf(key, sizeof(key), "Cost%d", i + 1);
            power->m_cost[i] = config.GetFloat(section, key, cost);
        }
        power->LoadConfig(config, section);
        power->SnapshotSettingDefaults(); // the Customize page's "Reset" goes back to these
        Log::Info("Power %-16s key=%s", power->Id(), Config::KeyName(power->m_key));
    }
}

void PowerManager::SetActive(Power& power, CPlayerPed* player, bool active) {
    if (power.m_active == active)
        return;
    // powers that drain energy need some in the tank to start
    if (active && power.m_drain[0] > 0.0f && !Energy::CanStart())
        return;
    if (active && power.ExclusiveGroup()) {
        for (auto& other : m_powers)
            if (other.get() != &power && other->m_active && other->ExclusiveGroup() &&
                std::strcmp(other->ExclusiveGroup(), power.ExclusiveGroup()) == 0)
                SetActive(*other, player, false);
    }
    if (!active && power.m_level > 0)
        power.m_lastLevel = power.m_level; // remembered for the shortcut key
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

Power* PowerManager::SetKey(Power& power, int vk) {
    Power* previousOwner = nullptr;
    if (vk) {
        for (auto& other : m_powers)
            if (other.get() != &power && other->m_key == vk) {
                other->m_key = 0; // one key, one power
                previousOwner = other.get();
            }
    }
    power.m_key = vk;
    return previousOwner;
}

void PowerManager::Fire(Power& power, CPlayerPed* player) {
    if (!power.PayForUse())
        return; // not enough energy (TrySpend already gave feedback)
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
    // real seconds, even in Bullet Time: energy shouldn't get cheaper in slow motion
    float timeScale = CTimer::ms_fTimeScale > 0.05f ? CTimer::ms_fTimeScale : 0.05f;
    float realDt = dt / timeScale;
    Energy::Update(realDt);

    for (auto& power : m_powers) {
        if (power->m_active && power->DrainPerSecond() > 0.0f && power->IsDraining() &&
            !Energy::Drain(power->DrainPerSecond() * realDt)) {
            SetActive(*power, player, false);
            static char message[64]; // static: CMessages keeps the pointer while it's on screen
            std::snprintf(message, sizeof(message), "~r~Out of energy:~s~ %s", power->Name());
            CMessages::AddMessageJumpQ(message, 2000, 0);
        }
        // hotkeys are paused while the menu is open (it uses the keyboard), but powers keep running
        int key = m_hotkeysEnabled ? power->m_key : 0;
        switch (power->GetMode()) {
        case Power::Mode::Toggle:
            // shortcut: off -> level 1 -> 2 -> ... -> max -> off (no levels: just on / off)
            if (Input::JustPressed(key)) {
                int next = power->m_active ? power->m_level + 1 : 1;
                SetLevel(*power, player, next > power->MaxLevel() ? 0 : next);
            }
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
