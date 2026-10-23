#include "powers/SuperSpeed.h"

#include "core/Config.h"
#include "core/Fx.h"
#include "core/Sound.h"
#include "core/Log.h"

#include "plugin.h"
#include "common.h" // RpAnimBlendClumpGetAssociation
#include "CAnimBlendAssociation.h"
#include "CPlayerInfo.h"
#include "CPlayerPed.h"
#include "CTimer.h"
#include "CWorld.h"

#include <cstdio>

namespace {

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
        std::snprintf(key, sizeof(key), "Jump%d", i + 1);
        m_jumpSpeed[i] = config.GetFloat(section, key, m_jumpSpeed[i]);
    }
    m_noFatigue = config.GetBool(section, "NoFatigue", true);
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
    if (player->bInVehicle) {
        EndJump(player);
        m_wasStanding = true;
        return;
    }

    if (m_noFatigue)
        if (CPlayerInfo* info = player->GetPlayerInfoForThisPlayerPed())
            info->m_bDoesNotGetTired = true;

    int lvl = Level() - 1; // array index
    if (!player->bApplyGravity) {
        // Flight is on (gravity off): it controls movement, don't push or launch him as well
        m_wasStanding = false;
        return;
    }
    UpdateJump(player, lvl);
    UpdateRun(player, lvl);
}

void SuperSpeed::UpdateRun(CPlayerPed* player, int lvl) {
    SetAnimSpeed(player, m_animSpeed[lvl]);

    if (player->m_nMoveState < PEDMOVE_WALK && !player->bIsInTheAir)
        return; // standing still / turning on the spot

    // m_vecMoveSpeed = distance moved per time step (1 step = 1/50 s), as computed by the game this frame
    CVector velocity = player->m_vecMoveSpeed;
    velocity.z = 0.0f; // horizontal only; the ground/gravity handle height
    CVector extra = velocity * ((m_multiplier[lvl] - 1.0f) * CTimer::ms_fTimeStep);
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
    if (m_wasStanding && !standing && player->m_vecMoveSpeed.z > kTakeOffMinZ && m_jumpSpeed[lvl] > 0.0f) {
        player->m_vecMoveSpeed.z = m_jumpSpeed[lvl];
        Sound::Play(Sound::Whoosh, 0.5f + 0.2f * lvl);
        if (!m_inSuperJump) {
            // big jumps = big landings: no fall damage until we touch the ground again
            m_wasCollisionProof = player->bCollisionProof;
            player->bCollisionProof = true;
            m_inSuperJump = true;
        }
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
    if (!m_inSuperJump)
        return;
    player->bCollisionProof = m_wasCollisionProof;
    m_inSuperJump = false;
}

void SuperSpeed::OnDeactivate(CPlayerPed* player) {
    SetAnimSpeed(player, 1.0f);
    EndJump(player);
    if (m_noFatigue)
        if (CPlayerInfo* info = player->GetPlayerInfoForThisPlayerPed())
            info->m_bDoesNotGetTired = m_wasTireless;
}
