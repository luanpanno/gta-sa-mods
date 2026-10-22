#pragma once

#include "core/Power.h"

// Phase 3.1 + 3.2: CJ runs several times faster on foot, and jumps higher, in 3 levels
// (key cycles off -> 1 fast -> 2 faster -> 3 super fast -> off).
//
// Speed: GTA SA moves peds by their animation ("root motion"), so there's no simple "speed" field.
// Each frame, after the game has moved CJ normally, we push him further in the same direction
// (Multiplier x his normal speed), checking with a line-of-sight test so he never goes through walls.
// The walk/run/sprint animations are also sped up so his legs match.
//
// Jump: when CJ leaves the ground going up (a real jump, not falling off a ledge), we replace his
// vertical speed with a bigger one. The speed push above keeps working in the air -> long jumps too.
class SuperSpeed : public Power {
public:
    static constexpr int kLevels = 3;

    const char* Name() const override { return "Super Speed"; }
    const char* Id() const override { return "SuperSpeed"; }
    int MaxLevel() const override { return kLevels; }

    void LoadConfig(const Config& config, const char* section) override;
    void OnActivate(CPlayerPed* player) override;
    void Update(CPlayerPed* player, float dt) override;
    void OnDeactivate(CPlayerPed* player) override;

private:
    void SetAnimSpeed(CPlayerPed* player, float speed);
    void UpdateRun(CPlayerPed* player, int lvl);
    void UpdateJump(CPlayerPed* player, int lvl);
    void EndJump(CPlayerPed* player);

    // per level (index 0 = level 1)
    float m_multiplier[kLevels] = {2.5f, 4.5f, 8.0f}; // ground speed vs normal (1 = normal)
    float m_animSpeed[kLevels] = {1.3f, 1.6f, 2.0f};  // leg animation speed (visual)
    float m_jumpSpeed[kLevels] = {0.25f, 0.40f, 0.60f}; // vertical take-off speed; 0 = normal jump

    bool m_noFatigue = true;
    bool m_wasTireless = false;   // CJ's "never tired" state before we turned it on, to restore it
    bool m_wasStanding = true;    // last frame's bIsStanding, to detect take-off
    bool m_inSuperJump = false;   // airborne from a boosted jump (we made him collision-proof)
    bool m_wasCollisionProof = false;
};
