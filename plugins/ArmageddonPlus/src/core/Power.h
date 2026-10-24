#pragma once

#include <vector>

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
    // Menu section: "Powers" (default) or "World" (gravity, chaos...: things that change the world, not CJ)
    virtual const char* Category() const { return "Powers"; }
    // Optional name per level, shown instead of "Level 2/3" (e.g. "Moon"). nullptr = numbers.
    virtual const char* LevelName(int level) const { return nullptr; }
    // One line shown at the bottom of the menu when the power is selected (controls, what it does).
    virtual const char* Description() const { return ""; }

    // Picker (OneShot powers like Spawn Vehicle): the menu shows PickerValue() and Left/Right call
    // PickerStep(-1 / +1), or -10 / +10 with Shift held. Enter fires the power with the current choice.
    virtual bool HasPicker() const { return false; }
    virtual const char* PickerValue() const { return ""; }
    virtual void PickerStep(int delta) {}

    // Read tuning values from [Power.<Id>]. Called once at startup.
    virtual void LoadConfig(const Config&, const char* section) {}

    virtual void OnActivate(CPlayerPed* player) {}   // Toggle/Hold: just turned on. OneShot: fire.
    virtual void OnDeactivate(CPlayerPed* player) {} // Toggle/Hold: just turned off (also on death)
    virtual void OnLevelChanged(CPlayerPed* player, int level) {} // after OnActivate, and on every level step
    virtual void Update(CPlayerPed* player, float dt) {} // every frame while active; dt in seconds
    virtual void UpdateAlways(CPlayerPed* player, float dt) {} // every frame, active or not (clean-up after turning off)
    virtual void Draw() {} // HUD drawing (screen effects etc.), every frame while active

    bool IsActive() const { return m_active; }
    int Level() const { return m_level; } // 0 when off, 1..MaxLevel() when on
    int LastLevel() const { return m_lastLevel; } // what Enter / the shortcut turns it back on at
    int Key() const { return m_key; }

    // Energy (see core/Energy.h). Values come from the ini: Drain1..3 / Cost1..3 (or Drain / Cost).
    static constexpr int kMaxLevels = 3;
    float DrainPerSecond() const; // while on, at the current level
    // false = on but idle right now (e.g. Super Speed while CJ stands still): no drain this frame
    virtual bool IsDraining() const { return true; }
    float CostPerUse() const;     // one action, at the current level
    // Call before an action (a blast, a strike, a grab...). false = not enough energy: skip the action.
    bool PayForUse() const;

    // Customize page (menu): tuning values the player can change in game, from Min to Max (Max = as
    // far as the game copes with). A power lists them once, in its constructor, with AddSetting().
    // They point at the same members LoadConfig fills, so a change applies right away.
    enum class Display {
        Plain,      // shown as stored
        Percent,    // stored 0..1, shown 0..100 %
        JumpHeight, // stored as take-off speed (m/step), shown as the height it reaches (m)
    };
    struct Setting {
        const char* label;  // "Run speed"
        const char* key;    // ini key: "Multiplier" -> Multiplier1..3 when per level, else just the key
        float* floats;      // storage (an array of kMaxLevels when per level), or nullptr
        int* ints;          // ... or an int one
        bool perLevel;
        float min, max, step; // in shown units
        const char* unit;   // "x", "m/s"...
        Display display;
        float defaults[kMaxLevels]; // stored values after the main ini loaded (for Reset)
    };
    int SettingCount() const { return (int)m_settings.size(); }
    const Setting& GetSetting(int index) const { return m_settings[index]; }
    bool HasPerLevelSettings() const;
    // level = 1..MaxLevel (ignored by settings that aren't per level)
    float SettingShown(int index, int level) const;
    void SetSettingShown(int index, int level, float shown); // clamped to min..max
    float SettingStored(int index, int level) const;
    void SetSettingStored(int index, int level, float stored);
    void SettingKey(int index, int level, char* out, int size) const; // ini key, e.g. "Multiplier3"
    void ResetSettings();
    void SnapshotSettingDefaults(); // after LoadConfig: what Reset goes back to

protected:
    void AddSetting(const char* label, const char* key, float* values, bool perLevel, float min, float max,
                    float step, const char* unit, Display display = Display::Plain);
    void AddSetting(const char* label, const char* key, int* values, bool perLevel, int min, int max,
                    const char* unit);

private:
    float* StoragePtr(int index, int level, int** intOut) const;
    std::vector<Setting> m_settings;

    friend class PowerManager;
    int LevelIndex() const; // 0-based, safe for OneShot powers (level 0)

    bool m_active = false;
    int m_level = 0;
    int m_lastLevel = 1; // level it had when last switched off: the shortcut brings it back at this level
    int m_key = 0;
    float m_drain[kMaxLevels] = {};
    float m_cost[kMaxLevels] = {};
};
