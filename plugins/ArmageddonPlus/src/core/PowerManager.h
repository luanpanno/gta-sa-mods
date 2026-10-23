#pragma once

#include "core/Power.h"

#include <memory>
#include <vector>

class Config;

// Owns all powers: binds their hotkeys, turns them on/off, ticks the active ones,
// switches everything off when CJ dies, and draws the debug overlay.
class PowerManager {
public:
    PowerManager() { s_instance = this; }
    static PowerManager& Instance() { return *s_instance; } // for powers that react to other powers

    // true if any power of this category is on (e.g. "Powers": is CJ using superpowers right now?)
    bool AnyActive(const char* category) const;

    void Register(std::unique_ptr<Power> power);
    void LoadConfig(const Config& config);

    void Update();     // call from Events::gameProcessEvent, after Input::Update()
    void DrawPowers();  // active powers' own HUD/screen effects; call from Events::drawHudEvent

    void DeactivateAll(CPlayerPed* player);

    // used by the menu
    size_t Count() const { return m_powers.size(); }
    Power& At(size_t index) { return *m_powers[index]; }
    void SetLevel(Power& power, CPlayerPed* player, int level); // 0 = off, clamped to MaxLevel()
    void Fire(Power& power, CPlayerPed* player);                // OneShot powers
    // Bind a shortcut (0 = none). Returns the power that had this key before (now unbound), if any.
    Power* SetKey(Power& power, int vk);
    void SetHotkeysEnabled(bool enabled) { m_hotkeysEnabled = enabled; }
    bool IsOverlayEnabled() const { return m_overlayEnabled; }
    void SetOverlayEnabled(bool enabled) { m_overlayEnabled = enabled; }

private:
    void SetActive(Power& power, CPlayerPed* player, bool active);

    static inline PowerManager* s_instance = nullptr;
    std::vector<std::unique_ptr<Power>> m_powers;
    bool m_overlayEnabled = true;
    bool m_hotkeysEnabled = true;
    int m_overlayKey = 0;
};
