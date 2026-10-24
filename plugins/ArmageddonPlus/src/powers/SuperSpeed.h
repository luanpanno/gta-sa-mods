#pragma once

#include "core/Power.h"

// Phase 3.1: CJ runs several times faster on foot, in 3 levels
// (key cycles off -> 1 fast -> 2 faster -> 3 super fast -> off).
//
// Speed: GTA SA moves peds by their animation ("root motion"), so there's no simple "speed" field.
// Each frame, after the game has moved CJ normally, we push him further in the same direction
// (Multiplier x his normal speed), checking with a line-of-sight test so he never goes through walls.
// The walk/run/sprint animations are also sped up so his legs match.
//
// Jump: normal height (higher jumps are the Super Jump power), but on take-off Leap holds his run
// speed (x JumpForward) so air drag doesn't slow him; the speed push above keeps working in the air
// -> long, fast running jumps, landing on his feet. With Super Jump also on, its leap replaces ours.
//
// Water: he swims faster (same push, SwimMultiplier). And on foot he WALKS ON WATER: GTA has no
// "solid water" switch, so we keep an invisible flat object (a51_jetdoor, a big slab) under his feet
// at the water surface; the game's normal collision lets him stand and run on it. If he's already
// swimming, it stays off (he swims fast instead).
class SuperSpeed : public Power {
public:
    static constexpr int kLevels = 3;

    const char* Name() const override { return "Super Speed"; }
    const char* Id() const override { return "SuperSpeed"; }
    const char* Description() const override { return "Run much faster and walk on water; running jumps keep the speed"; }
    int MaxLevel() const override { return kLevels; }

    void LoadConfig(const Config& config, const char* section) override;
    void OnActivate(CPlayerPed* player) override;
    void Update(CPlayerPed* player, float dt) override;
    void OnDeactivate(CPlayerPed* player) override;
    bool IsDraining() const override { return m_moving; } // standing still costs nothing

private:
    void SetAnimSpeed(CPlayerPed* player, float speed);
    void UpdateRun(CPlayerPed* player, int lvl);
    void UpdateJump(CPlayerPed* player, int lvl);
    void EndJump(CPlayerPed* player);
    void UpdateWaterWalk(CPlayerPed* player);
    void RemovePlatform();

    // per level (index 0 = level 1)
    float m_multiplier[kLevels] = {2.5f, 4.5f, 8.0f}; // ground speed vs normal (1 = normal)
    float m_animSpeed[kLevels] = {1.3f, 1.6f, 2.0f};  // leg animation speed (visual)
    float m_jumpForward[kLevels] = {1.2f, 1.35f, 1.5f};  // run speed kept in the air, x this (no air drag)
    float m_swimMultiplier[kLevels] = {2.0f, 3.5f, 6.0f}; // swim speed vs normal (surface and underwater)

    bool m_noFatigue = true;
    bool m_waterWalk = true;

    // water walk: an invisible flat object kept under his feet at the water surface
    int m_platform = 0;           // script handle (0 = none)
    float m_platformTop = 0.0f;   // height of its top surface above its origin
    bool m_platformFailed = false; // model didn't load: don't retry every frame
    bool m_wasTireless = false;   // CJ's "never tired" state before we turned it on, to restore it
    bool m_wasStanding = true;    // last frame's bIsStanding, to detect take-off
    bool m_inSuperJump = false;   // airborne from a boosted jump (for the landing dust)
    bool m_moving = false;        // running/walking or airborne this frame (drives energy drain)
};
