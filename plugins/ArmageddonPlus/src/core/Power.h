#pragma once

class CPlayerPed;
class Config;

// Base class for every superpower. A power only describes WHAT it does;
// PowerManager decides WHEN (hotkeys, death, HUD).
//
// To add a power: subclass this in src/powers/, then register it in main.cpp.
class Power {
public:
    enum class Mode {
        Toggle,  // press: on, press again: off       (flight, super speed)
        Hold,    // active while the key is held       (telekinesis beam)
        OneShot, // press: fire once                   (explosion punch, spawn)
    };

    virtual ~Power() = default;

    virtual const char* Name() const = 0; // shown on screen, e.g. "Super Speed"
    virtual const char* Id() const = 0;   // ini section is [Power.<Id>], e.g. "SuperSpeed"
    virtual Mode GetMode() const { return Mode::Toggle; }
    // Powers are driven from the menu. A hotkey is optional (Key= in the ini); 0 = none.
    virtual int DefaultKey() const { return 0; }
    // Toggle powers with MaxLevel() > 1: each key press steps off -> 1 -> 2 -> ... -> MaxLevel -> off.
    virtual int MaxLevel() const { return 1; }
    // Powers sharing a group can't be on together: turning one on turns the others off
    // (e.g. "mouse" = powers that take over the mouse buttons). nullptr = no group.
    virtual const char* ExclusiveGroup() const { return nullptr; }

    // Read tuning values from [Power.<Id>]. Called once at startup.
    virtual void LoadConfig(const Config&, const char* section) {}

    virtual void OnActivate(CPlayerPed* player) {}   // Toggle/Hold: just turned on. OneShot: fire.
    virtual void OnDeactivate(CPlayerPed* player) {} // Toggle/Hold: just turned off (also on death)
    virtual void OnLevelChanged(CPlayerPed* player, int level) {} // after OnActivate, and on every level step
    virtual void Update(CPlayerPed* player, float dt) {} // every frame while active; dt in seconds
    virtual void UpdateAlways(CPlayerPed* player, float dt) {} // every frame, active or not (clean-up after turning off)

    bool IsActive() const { return m_active; }
    int Level() const { return m_level; } // 0 when off, 1..MaxLevel() when on
    int Key() const { return m_key; }

private:
    friend class PowerManager;
    bool m_active = false;
    int m_level = 0;
    int m_key = 0;
};
