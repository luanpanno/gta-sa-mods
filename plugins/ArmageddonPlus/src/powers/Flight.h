#pragma once

#include "core/Power.h"
#include "CVector.h"

#include <string>

// Phase 3.4: CJ flies, Superman-style. 3 levels = cruise / fast / very fast.
//
//   W/S        fly toward / away from where the camera looks (look up + W = climb)
//   A/D        strafe
//   SPACE / C  straight up / down (configurable)
//   no keys    hover in place
//
// How: gravity is switched off for CJ (CPhysical::bApplyGravity) and every frame we set his
// velocity (m_vecMoveSpeed) to where the keys point. The game's own physics then moves him,
// including collisions with buildings, so he can't fly through walls. He's collision-proof
// while flying and until he next touches the ground (no fall/crash damage).
//
// Look: the game has no flying animations, so we borrow two. Moving = the skydiving head-first
// dive (FALL_SkyDive_Accel from parachute.ifp), hovering = treading water (Swim_Tread from ped.ifp),
// played with the same script task CLEO's task_play_anim uses.
class Flight : public Power {
public:
    static constexpr int kLevels = 3;

    const char* Name() const override { return "Flight"; }
    const char* Id() const override { return "Flight"; }
    int MaxLevel() const override { return kLevels; }

    void LoadConfig(const Config& config, const char* section) override;
    void OnActivate(CPlayerPed* player) override;
    void Update(CPlayerPed* player, float dt) override;
    void OnDeactivate(CPlayerPed* player) override;
    void UpdateAlways(CPlayerPed* player, float dt) override;

private:
    void StopFlying(CPlayerPed* player); // gravity back on, keep crash protection until landing
    void UpdatePose(CPlayerPed* player);  // dive while moving, tread while hovering

    struct Anim {
        std::string name;
        std::string file; // anim block: "PED" is always loaded, others are requested on activate
    };
    Anim m_flyAnim{"FALL_SkyDive_Accel", "PARACHUTE"};
    Anim m_hoverAnim{"Swim_Tread", "PED"};
    bool m_usePoses = true;
    bool m_diving = false; // current pose: true = fly anim, false = hover anim

    float m_speed[kLevels] = {12.0f, 30.0f, 60.0f};   // m/s forward
    float m_climb[kLevels] = {8.0f, 18.0f, 35.0f};    // m/s up/down with SPACE/C
    float m_responsiveness = 4.0f; // how quickly velocity follows the keys (higher = snappier)
    bool m_followCameraPitch = true;
    int m_ascendKey = 0;
    int m_descendKey = 0;

    CVector m_velocity;            // current velocity, m/s
    bool m_flying = false;         // we turned gravity off
    bool m_landingGuard = false;   // collision-proof until CJ touches the ground
    bool m_wasCollisionProof = false;
};
