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
    virtual int DefaultKey() const { return 0; } // VK code used if the ini doesn't set Key=

    // Read tuning values from [Power.<Id>]. Called once at startup.
    virtual void LoadConfig(const Config&, const char* section) {}

    virtual void OnActivate(CPlayerPed* player) {}   // Toggle/Hold: just turned on. OneShot: fire.
    virtual void OnDeactivate(CPlayerPed* player) {} // Toggle/Hold: just turned off (also on death)
    virtual void Update(CPlayerPed* player, float dt) {} // every frame while active; dt in seconds

    bool IsActive() const { return m_active; }
    int Key() const { return m_key; }

private:
    friend class PowerManager;
    bool m_active = false;
    int m_key = 0;
};
