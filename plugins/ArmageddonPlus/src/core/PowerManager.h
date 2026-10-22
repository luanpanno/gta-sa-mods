#pragma once

#include "core/Power.h"

#include <memory>
#include <vector>

class Config;

// Owns all powers: binds their hotkeys, turns them on/off, ticks the active ones,
// switches everything off when CJ dies, and draws the debug overlay.
class PowerManager {
public:
    void Register(std::unique_ptr<Power> power);
    void LoadConfig(const Config& config);

    void Update();     // call from Events::gameProcessEvent, after Input::Update()
    void DrawOverlay(); // call from Events::drawHudEvent
    void DrawPowers();  // active powers' own HUD/screen effects; call from Events::drawHudEvent

    void DeactivateAll(CPlayerPed* player);

    // used by the menu
    size_t Count() const { return m_powers.size(); }
    Power& At(size_t index) { return *m_powers[index]; }
    void SetLevel(Power& power, CPlayerPed* player, int level); // 0 = off, clamped to MaxLevel()
    void Fire(Power& power, CPlayerPed* player);                // OneShot powers
    void SetHotkeysEnabled(bool enabled) { m_hotkeysEnabled = enabled; }
    bool IsOverlayEnabled() const { return m_overlayEnabled; }
    void SetOverlayEnabled(bool enabled) { m_overlayEnabled = enabled; }

private:
    void SetActive(Power& power, CPlayerPed* player, bool active);
    void StepLevel(Power& power, CPlayerPed* player); // Toggle key press: off -> 1 -> ... -> max -> off

    std::vector<std::unique_ptr<Power>> m_powers;
    bool m_overlayEnabled = true;
    bool m_hotkeysEnabled = true;
    int m_overlayKey = 0;
};
