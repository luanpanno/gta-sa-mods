#pragma once

#include "core/Power.h"

// Super Jump on its own (no Super Speed needed). Jump normally (Shift by default); levels:
//   1 High    ~30 m up, longer
//   2 Higher  ~83 m up, much longer
//   3 Sky     ~130 m up, huge distance, and landing is a GROUND POUND (shockwave)
//
// How: when CJ leaves the ground going up (a real jump, not a fall), Leap takes over: it drives a
// clean ballistic arc with take-off speed JumpN, holding his horizontal speed x ForwardN, and
// FallGuard keeps him from taking fall damage until he lands.
class SuperJump : public Power {
public:
    static constexpr int kLevels = 3;

    SuperJump() {
        AddSetting("Height", "Jump", m_jump, true, 5.0f, 400.0f, 5.0f, "m", Display::JumpHeight); // Customize page
        AddSetting("Distance", "Forward", m_forward, true, 1.0f, 8.0f, 0.1f, "x");
        AddSetting("Ground pound radius", "PoundRadius", m_poundRadius, true, 0.0f, 60.0f, 1.0f, "m");
        AddSetting("Ground pound force", "PoundSpeed", m_poundSpeed, true, 0.0f, 150.0f, 5.0f, "m/s");
    }

    const char* Name() const override { return "Super Jump"; }
    const char* Id() const override { return "SuperJump"; }
    const char* Description() const override { return "Jump as usual: much higher and longer; level 3 lands with a ground-pound shockwave"; }
    int MaxLevel() const override { return kLevels; }
    const char* LevelName(int level) const override;

    void LoadConfig(const Config& config, const char* section) override;
    void OnActivate(CPlayerPed* player) override;
    void Update(CPlayerPed* player, float dt) override;

private:
    float m_jump[kLevels] = {0.70f, 1.15f, 1.45f};    // take-off speed (m per step): ~30 / 83 / 130 m high
    float m_forward[kLevels] = {1.5f, 2.2f, 3.0f};    // horizontal speed multiplier at take-off
    float m_poundRadius[kLevels] = {0.0f, 10.0f, 20.0f}; // ground-pound shockwave (0 = none)
    float m_poundSpeed[kLevels] = {0.0f, 20.0f, 40.0f};

    bool m_wasStanding = true;
    bool m_inJump = false;
    float m_airTime = 0.0f;
};
