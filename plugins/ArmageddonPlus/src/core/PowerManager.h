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

    void DeactivateAll(CPlayerPed* player);

private:
    void SetActive(Power& power, CPlayerPed* player, bool active);
    void StepLevel(Power& power, CPlayerPed* player); // Toggle key press: off -> 1 -> ... -> max -> off

    std::vector<std::unique_ptr<Power>> m_powers;
    bool m_overlayEnabled = true;
    int m_overlayKey = 0;
};
