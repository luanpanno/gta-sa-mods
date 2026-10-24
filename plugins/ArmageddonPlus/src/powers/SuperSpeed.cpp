#include "powers/SuperSpeed.h"

#include "core/Config.h"
#include "core/Leap.h"
#include "core/Fx.h"
#include "core/Log.h"

#include "plugin.h"
#include "common.h" // RpAnimBlendClumpGetAssociation
#include "CAnimBlendAssociation.h"
#include "CPlayerInfo.h"
#include "CPlayerPed.h"
#include "CPedIntelligence.h"
#include "CTaskSimpleSwim.h"
#include "CTimer.h"
#include "CWorld.h"
#include "CModelInfo.h"
#include "CStreaming.h"
#include "CWaterLevel.h"
#include "extensions/ScriptCommands.h"

#include <cstdio>

namespace {

// a51_jetdoor: a big flat slab with simple collision, a common choice for invisible platforms
const int kPlatformModel = 3095;

// Animation IDs shared by every on-foot animation group (player, fat, muscle, gun-holding...)
const unsigned int kMoveAnims[] = {0 /* walk */, 1 /* run */, 2 /* sprint */};

// A normal jump leaves the ground at roughly 0.1 units/step upwards; walking off a ledge is <= 0.
const float kTakeOffMinZ = 0.03f;

// True if nothing solid (buildings, vehicles, objects) is between a and b. Peds are ignored:
// we run through them and the game's collision shoves them aside.
bool IsPathClear(const CVector& a, const CVector& b) {
    return CWorld::GetIsLineOfSightClear(a, b, true, true, false, true, false, false, false);
}

} // namespace

void SuperSpeed::LoadConfig(const Config& config, const char* section) {
    char key[32];
    for (int i = 0; i < kLevels; ++i) {
        std::snprintf(key, sizeof(key), "Multiplier%d", i + 1);
        m_multiplier[i] = config.GetFloat(section, key, m_multiplier[i]);
        std::snprintf(key, sizeof(key), "AnimSpeed%d", i + 1);
        m_animSpeed[i] = config.GetFloat(section, key, m_animSpeed[i]);
        std::snprintf(key, sizeof(key), "JumpForward%d", i + 1);
        m_jumpForward[i] = config.GetFloat(section, key, m_jumpForward[i]);
        std::snprintf(key, sizeof(key), "SwimMultiplier%d", i + 1);
        m_swimMultiplier[i] = config.GetFloat(section, key, m_swimMultiplier[i]);
    }
    m_noFatigue = config.GetBool(section, "NoFatigue", true);
    m_waterWalk = config.GetBool(section, "WaterWalk", true);
}

void SuperSpeed::OnActivate(CPlayerPed* player) {
    if (CPlayerInfo* info = player->GetPlayerInfoForThisPlayerPed())
        m_wasTireless = info->m_bDoesNotGetTired;
    m_wasStanding = player->bIsStanding;
    m_inSuperJump = false;
}

void SuperSpeed::SetAnimSpeed(CPlayerPed* player, float speed) {
    if (!player->m_pRwClump)
        return;
    for (unsigned int animId : kMoveAnims)
        if (CAnimBlendAssociation* anim = RpAnimBlendClumpGetAssociation(player->m_pRwClump, animId))
            anim->m_fSpeed = speed;
}

void SuperSpeed::Update(CPlayerPed* player, float) {
    m_moving = !player->bInVehicle && (player->m_nMoveState >= PEDMOVE_WALK || player->bIsInTheAir ||
                                       (player->bTouchingWater && player->m_vecMoveSpeed.Magnitude() > 0.01f));
    if (player->bInVehicle) {
        EndJump(player);
        RemovePlatform();
        m_wasStanding = true;
        return;
    }

    if (m_noFatigue)
        if (CPlayerInfo* info = player->GetPlayerInfoForThisPlayerPed())
            info->m_bDoesNotGetTired = true;

    int lvl = Level() - 1; // array index
    if (!player->bApplyGravity) {
        // Flight is on (gravity off): it controls movement, don't push or launch him as well
        RemovePlatform();
        m_wasStanding = false;
        return;
    }
    UpdateJump(player, lvl);
    UpdateRun(player, lvl);
    UpdateWaterWalk(player);
}

void SuperSpeed::UpdateWaterWalk(CPlayerPed* player) {
    if (!m_waterWalk || m_platformFailed)
        return;

    CTaskSimpleSwim* swim = player->m_pIntelligence ? player->m_pIntelligence->GetTaskSwim() : nullptr;
    CVector pos = player->GetPosition(); // ped position = about 1 m above his feet
    float water = 0.0f;
    bool overWater = !swim && CWaterLevel::GetWaterLevelNoWaves(pos.x, pos.y, pos.z, &water);
    if (overWater) {
        // real water, not a water plane hidden under the land: the ground must be below the surface,
        // and his feet near it (on it, or coming down onto it from a jump)
        bool found = false;
        float ground = CWorld::FindGroundZFor3DCoord(pos.x, pos.y, pos.z, &found, nullptr);
        float feetAbove = pos.z - 1.0f - water;
        overWater = (!found || ground < water - 0.3f) && feetAbove > -0.5f && feetAbove < 3.0f;
    }
    if (!overWater) {
        RemovePlatform();
        return;
    }

    using namespace plugin;
    if (m_platform && !Command<Commands::DOES_OBJECT_EXIST>(m_platform))
        m_platform = 0; // the game removed it (e.g. a save was loaded)
    if (!m_platform) {
        CStreaming::RequestModel(kPlatformModel, GAME_REQUIRED);
        CStreaming::LoadAllRequestedModels(false);
        if (CStreaming::ms_aInfoForModel[kPlatformModel].m_nLoadState != LOADSTATE_LOADED) {
            Log::Info("SuperSpeed: water-walk platform model %d didn't load; water walking off", kPlatformModel);
            m_platformFailed = true;
            return;
        }
        Command<Commands::CREATE_OBJECT>(kPlatformModel, pos.x, pos.y, water - 10.0f, &m_platform);
        Command<Commands::SET_OBJECT_VISIBLE>(m_platform, false);
        Command<Commands::SET_OBJECT_DYNAMIC>(m_platform, false); // never pushed around
        Command<Commands::SET_OBJECT_COLLISION>(m_platform, true);
        Command<Commands::MARK_MODEL_AS_NO_LONGER_NEEDED>(kPlatformModel);

        // lay it flat whatever way the model is built: if its thinnest side isn't up, rotate it
        m_platformTop = 0.0f;
        if (CBaseModelInfo* info = CModelInfo::GetModelInfo(kPlatformModel))
            if (CColModel* col = info->m_pColModel) {
                CVector size = col->m_boundBox.m_vecMax - col->m_boundBox.m_vecMin;
                Log::Info("SuperSpeed: platform size %.1f x %.1f x %.1f", size.x, size.y, size.z);
                if (size.z <= size.x && size.z <= size.y) {
                    m_platformTop = col->m_boundBox.m_vecMax.z;
                } else if (size.y <= size.x) {
                    Command<Commands::SET_OBJECT_ROTATION>(m_platform, 90.0f, 0.0f, 0.0f); // y becomes up
                    m_platformTop = col->m_boundBox.m_vecMax.y;
                } else {
                    Command<Commands::SET_OBJECT_ROTATION>(m_platform, 0.0f, 90.0f, 0.0f); // x becomes up
                    m_platformTop = -col->m_boundBox.m_vecMin.x;
                }
            }
    }
    // top surface exactly at the water surface, centered under him
    Command<Commands::SET_OBJECT_COORDINATES>(m_platform, pos.x, pos.y, water - m_platformTop);
}

void SuperSpeed::RemovePlatform() {
    if (!m_platform)
        return;
    if (plugin::Command<plugin::Commands::DOES_OBJECT_EXIST>(m_platform))
        plugin::Command<plugin::Commands::DELETE_OBJECT>(m_platform);
    m_platform = 0;
}

void SuperSpeed::UpdateRun(CPlayerPed* player, int lvl) {
    SetAnimSpeed(player, m_animSpeed[lvl]);

    // swimming is a task of its own (CTaskSimpleSwim), moved by its animations like running is,
    // so the same push works in water, with its own multiplier
    CTaskSimpleSwim* swim = player->m_pIntelligence ? player->m_pIntelligence->GetTaskSwim() : nullptr;
    if (swim) {
        if (player->m_pRwClump)
            if (CAnimBlendAssociation* anim = RpAnimBlendClumpGetAssociation(player->m_pRwClump, swim->m_AnimID))
                anim->m_fSpeed = m_animSpeed[lvl]; // strokes match the speed (visual only)
    } else if (player->m_nMoveState < PEDMOVE_WALK && !player->bIsInTheAir) {
        return; // standing still / turning on the spot
    }

    // m_vecMoveSpeed = distance moved per time step (1 step = 1/50 s), as computed by the game this frame
    CVector velocity = player->m_vecMoveSpeed;
    bool underwater = swim && swim->m_nSwimState >= SWIM_DIVE_UNDERWATER;
    if (!underwater)
        velocity.z = 0.0f; // horizontal only; the ground/gravity/water surface handle height
    float multiplier = swim ? m_swimMultiplier[lvl] : m_multiplier[lvl];
    CVector extra = velocity * ((multiplier - 1.0f) * CTimer::ms_fTimeStep);
    if (extra.Magnitude() < 0.001f)
        return;

    CVector from = player->GetPosition(); // ped position = roughly his waist
    CVector to = from + extra;
    CVector lookAhead = extra.Normalized() * 0.5f; // stop half a meter before walls
    CVector knee(0.0f, 0.0f, -0.6f);

    if (IsPathClear(from, to + lookAhead) && IsPathClear(from + knee, to + lookAhead + knee))
        player->SetPosn(to);
}

void SuperSpeed::UpdateJump(CPlayerPed* player, int lvl) {
    bool standing = player->bIsStanding;

    // take-off: was on the ground last frame, now airborne and going up
    if (m_wasStanding && !standing && player->m_vecMoveSpeed.z > kTakeOffMinZ) {
        // a normal-height jump (height is Super Jump's job), but driven by Leap (see core/Leap.h) so it
        // holds his take-off run speed (x JumpForward): otherwise air drag slows him mid-jump, and
        // UpdateRun's push (a multiple of that speed) shrinks with it. Leap also gives the soft landing.
        CVector horizontal(player->m_vecMoveSpeed.x * m_jumpForward[lvl],
                           player->m_vecMoveSpeed.y * m_jumpForward[lvl], 0.0f);
        Leap::Start(player, player->m_vecMoveSpeed.z, horizontal);
        m_inSuperJump = true;
    }

    if (standing && m_inSuperJump) {
        // landing: a burst of dust at his feet
        CVector feet = player->GetPosition();
        feet.z -= 1.0f;
        Fx::Spawn("wallbust", feet);
        EndJump(player);
    }

    m_wasStanding = standing;
}

void SuperSpeed::EndJump(CPlayerPed* player) {
    m_inSuperJump = false; // FallGuard restores his collision state when he lands
}

void SuperSpeed::OnDeactivate(CPlayerPed* player) {
    SetAnimSpeed(player, 1.0f);
    EndJump(player);
    RemovePlatform();
    if (m_noFatigue)
        if (CPlayerInfo* info = player->GetPlayerInfoForThisPlayerPed())
            info->m_bDoesNotGetTired = m_wasTireless;
}
