#pragma once

#include "core/Power.h"

// Super Jump on its own (no Super Speed needed). Jump normally (Shift by default); levels:
//   1 High    ~15 m up, a bit longer
//   2 Higher  ~40 m up, longer
//   3 Sky     ~100 m up, much longer, and landing is a GROUND POUND (shockwave)
//
// How: same trick as Super Speed's jump. When CJ leaves the ground going up (a real jump, not a
// fall), his vertical speed is replaced with JumpN and his horizontal speed multiplied by ForwardN.
// FallGuard keeps him from taking fall damage until he lands.
class SuperJump : public Power {
public:
    static constexpr int kLevels = 3;

    const char* Name() const override { return "Super Jump"; }
    const char* Id() const override { return "SuperJump"; }
    const char* Description() const override { return "Jump as usual: much higher and longer; level 3 lands with a ground-pound shockwave"; }
    int MaxLevel() const override { return kLevels; }
    const char* LevelName(int level) const override;

    void LoadConfig(const Config& config, const char* section) override;
    void OnActivate(CPlayerPed* player) override;
    void Update(CPlayerPed* player, float dt) override;

private:
    float m_jump[kLevels] = {0.55f, 0.90f, 1.40f};    // vertical take-off speed (m per step)
    float m_forward[kLevels] = {1.5f, 2.2f, 3.0f};    // horizontal speed multiplier at take-off
    float m_poundRadius[kLevels] = {0.0f, 0.0f, 12.0f}; // ground-pound shockwave (0 = none)
    float m_poundSpeed[kLevels] = {0.0f, 0.0f, 25.0f};

    bool m_wasStanding = true;
    bool m_inJump = false;
    float m_airTime = 0.0f;
};
