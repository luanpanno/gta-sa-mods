#include "powers/Flight.h"

#include "core/Config.h"
#include "core/Fx.h"
#include "core/Input.h"
#include "core/Sound.h"

#include "plugin.h"
#include "CCamera.h"
#include "CPad.h"
#include "CPlayerPed.h"
#include "CPools.h"
#include "CTimer.h"
#include "CWorld.h"
#include "extensions/ScriptCommands.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

using namespace plugin;

namespace {

// m_vecMoveSpeed is in units (meters) per time step; one step = 1/50 s
const float kStepsPerSecond = 50.0f;

// pose switching, with a gap so it doesn't flicker around one speed
const float kDiveAboveSpeed = 4.0f;  // m/s
const float kHoverBelowSpeed = 2.0f; // m/s

bool IsPedFile(const std::string& file) {
    return _stricmp(file.c_str(), "PED") == 0;
}

} // namespace

void Flight::LoadConfig(const Config& config, const char* section) {
    char key[32];
    for (int i = 0; i < kLevels; ++i) {
        std::snprintf(key, sizeof(key), "Speed%d", i + 1);
        m_speed[i] = config.GetFloat(section, key, m_speed[i]);
        std::snprintf(key, sizeof(key), "Climb%d", i + 1);
        m_climb[i] = config.GetFloat(section, key, m_climb[i]);
    }
    m_responsiveness = config.GetFloat(section, "Responsiveness", m_responsiveness);
    m_followCameraPitch = config.GetBool(section, "FollowCameraPitch", true);
    m_ascendKey = config.GetKey(section, "AscendKey", VK_SPACE);
    m_descendKey = config.GetKey(section, "DescendKey", 'C');
    m_usePoses = config.GetBool(section, "Poses", true);
    m_flyAnim.name = config.GetText(section, "FlyAnim", m_flyAnim.name.c_str());
    m_flyAnim.file = config.GetText(section, "FlyAnimFile", m_flyAnim.file.c_str());
    m_hoverAnim.name = config.GetText(section, "HoverAnim", m_hoverAnim.name.c_str());
    m_hoverAnim.file = config.GetText(section, "HoverAnimFile", m_hoverAnim.file.c_str());
}

void Flight::OnActivate(CPlayerPed* player) {
    // non-PED anim files live in anim.img and must be streamed in before use
    if (m_usePoses) {
        if (!IsPedFile(m_flyAnim.file))
            Command<Commands::REQUEST_ANIMATION>(m_flyAnim.file.c_str());
        if (!IsPedFile(m_hoverAnim.file))
            Command<Commands::REQUEST_ANIMATION>(m_hoverAnim.file.c_str());
    }
    m_diving = false;
    m_velocity = CVector(0.0f, 0.0f, 0.0f);
    m_climbExpected = 0.0f;
    if (!m_landingGuard)
        m_wasCollisionProof = player->bCollisionProof;
    m_landingGuard = false;
}

void Flight::Update(CPlayerPed* player, float dt) {
    if (player->bInVehicle) {
        StopFlying(player);
        return;
    }

    if (!m_flying) {
        // take off: lift CJ off the ground so the game treats him as airborne
        player->bApplyGravity = false;
        player->bCollisionProof = true;
        CVector pos = player->GetPosition();
        if (player->bIsStanding)
            player->SetPosn(pos.x, pos.y, pos.z + 0.5f);
        m_flying = true;
        Sound::Play(Sound::Whoosh, 0.8f);
        CVector ground;
        if (Fx::GroundBelow(pos, 3.0f, ground))
            Fx::Spawn("heli_dust", ground);
    }

    // downdraft: dust kicked up under CJ while he flies low (like a helicopter)
    m_dustTimer -= dt;
    if (m_dustTimer <= 0.0f) {
        m_dustTimer = 0.35f;
        CVector ground;
        if (Fx::GroundBelow(player->GetPosition(), 8.0f, ground))
            Fx::Spawn("heli_dust", ground);
    }
    player->bApplyGravity = false; // re-applied every frame, the game can reset it
    player->bCollisionProof = true;

    // climbing: the game damps a ped's upward speed (peds aren't built to rise), so a fast climb came
    // out far slower than Climb/Speed asked. Measure how much he actually rose since last frame and
    // make up the rest by moving him, unless something solid is above him.
    CVector pos = player->GetPosition();
    if (m_climbExpected > 0.0f) {
        float missing = std::min(m_climbExpected - (pos.z - m_lastZ), m_climbExpected);
        CVector to = pos + CVector(0.0f, 0.0f, missing);
        if (missing > 0.01f &&
            CWorld::GetIsLineOfSightClear(pos, to + CVector(0.0f, 0.0f, 1.0f), true, true, false, true, false, false, false))
            player->SetPosn(to);
    }

    // --- where the keys point ----------------------------------------------------------
    CPad* pad = CPad::GetPad(0);
    // walk axes: -128..128; forward (W) is negative
    float forward = pad ? -pad->GetPedWalkUpDown() / 128.0f : 0.0f;
    float strafe = pad ? pad->GetPedWalkLeftRight() / 128.0f : 0.0f;
    float vertical = (Input::IsDown(m_ascendKey) ? 1.0f : 0.0f) - (Input::IsDown(m_descendKey) ? 1.0f : 0.0f);

    CVector front = TheCamera.m_aCams[TheCamera.m_nActiveCam].m_vecFront;
    if (!m_followCameraPitch)
        front.z = 0.0f;
    front.Normalize();
    CVector flatFront(front.x, front.y, 0.0f);
    flatFront.Normalize();
    CVector right(flatFront.y, -flatFront.x, 0.0f); // 90 degrees clockwise from the camera direction

    int lvl = Level() - 1;
    CVector target = (front * forward + right * strafe) * m_speed[lvl];
    target.z += vertical * m_climb[lvl];

    // game time may be slowed (Bullet Time): CJ's flight should still follow real time
    float timeScale = CTimer::ms_fTimeScale > 0.05f ? CTimer::ms_fTimeScale : 0.05f;
    float realDt = dt / timeScale;

    // ease toward the target velocity (frame-rate independent)
    float blend = 1.0f - std::exp(-m_responsiveness * realDt);
    m_velocity += (target - m_velocity) * blend;

    // the game moves CJ by this velocity during the next physics update, with collisions.
    // Physics steps are shortened by the time scale, so scale the velocity up to compensate.
    player->m_vecMoveSpeed = m_velocity / (kStepsPerSecond * timeScale);
    m_lastZ = player->GetPosition().z;
    m_climbExpected = m_velocity.z > 0.0f ? m_velocity.z * realDt : 0.0f; // checked next frame

    // face the direction of travel
    if (m_velocity.Magnitude2D() > 1.0f) {
        float heading = std::atan2(-m_velocity.x, m_velocity.y);
        player->m_fHeadingCurrent = heading;
        player->m_fHeadingGoal = heading;
        player->SetHeading(heading);
    }

    UpdatePose(player);
}

void Flight::UpdatePose(CPlayerPed* player) {
    if (!m_usePoses)
        return;

    float speed = m_velocity.Magnitude();
    if (m_diving && speed < kHoverBelowSpeed)
        m_diving = false;
    else if (!m_diving && speed > kDiveAboveSpeed)
        m_diving = true;

    const Anim& anim = m_diving ? m_flyAnim : m_hoverAnim;
    if (!IsPedFile(anim.file) && !Command<Commands::HAS_ANIMATION_LOADED>(anim.file.c_str()))
        return; // still streaming in; the default falling pose shows meanwhile

    int handle = CPools::GetPedRef(player);
    // (re)start the pose if it isn't playing: first frame, after switching, or if the game interrupted it
    if (!Command<Commands::IS_CHAR_PLAYING_ANIM>(handle, anim.name.c_str())) {
        // blend 4.0 = smooth ~quarter-second crossfade; loop; don't move him by the anim (lockX/Y off)
        Command<Commands::TASK_PLAY_ANIM_NON_INTERRUPTABLE>(handle, anim.name.c_str(), anim.file.c_str(), 4.0f,
                                                            true, false, false, false, -1);
    }
}

void Flight::StopFlying(CPlayerPed* player) {
    if (!m_flying)
        return;
    player->bApplyGravity = true;
    m_flying = false;
    m_climbExpected = 0.0f;
    if (m_usePoses) {
        // drop the pose so the game's normal falling/landing takes over
        // (not when dead or in a car: clearing tasks there would mess with dying / driving)
        if (player->m_fHealth > 0.0f && !player->bInVehicle)
            Command<Commands::CLEAR_CHAR_TASKS>(CPools::GetPedRef(player));
        if (!IsPedFile(m_flyAnim.file))
            Command<Commands::REMOVE_ANIMATION>(m_flyAnim.file.c_str());
        if (!IsPedFile(m_hoverAnim.file))
            Command<Commands::REMOVE_ANIMATION>(m_hoverAnim.file.c_str());
    }
    m_landingGuard = true; // stays collision-proof until he lands (see UpdateAlways)
}

void Flight::OnDeactivate(CPlayerPed* player) {
    StopFlying(player);
    // keep some of the momentum, minus the upward part, so he doesn't get launched
    if (player->m_vecMoveSpeed.z > 0.0f)
        player->m_vecMoveSpeed.z = 0.0f;
}

void Flight::UpdateAlways(CPlayerPed* player, float) {
    if (!m_landingGuard || IsActive())
        return;
    if (player->bIsStanding || player->bInVehicle || player->bTouchingWater) {
        player->bCollisionProof = m_wasCollisionProof;
        m_landingGuard = false;
    }
}
