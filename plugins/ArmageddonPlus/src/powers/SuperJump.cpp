#include "powers/SuperJump.h"

#include "core/Config.h"
#include "core/FallGuard.h"
#include "core/Fx.h"
#include "core/Shockwave.h"
#include "core/Sound.h"

#include "plugin.h"
#include "CPlayerPed.h"

#include <cstdio>

namespace {
// a normal jump leaves the ground at roughly 0.1 m/step upward; walking off a ledge is <= 0
const float kTakeOffMinZ = 0.03f;
}

const char* SuperJump::LevelName(int level) const {
    static const char* kNames[kLevels] = {"High", "Higher", "Sky"};
    return level >= 1 && level <= kLevels ? kNames[level - 1] : nullptr;
}

void SuperJump::LoadConfig(const Config& config, const char* section) {
    char key[32];
    for (int i = 0; i < kLevels; ++i) {
        std::snprintf(key, sizeof(key), "Jump%d", i + 1);
        m_jump[i] = config.GetFloat(section, key, m_jump[i]);
        std::snprintf(key, sizeof(key), "Forward%d", i + 1);
        m_forward[i] = config.GetFloat(section, key, m_forward[i]);
        std::snprintf(key, sizeof(key), "PoundRadius%d", i + 1);
        m_poundRadius[i] = config.GetFloat(section, key, m_poundRadius[i]);
        std::snprintf(key, sizeof(key), "PoundSpeed%d", i + 1);
        m_poundSpeed[i] = config.GetFloat(section, key, m_poundSpeed[i]);
    }
}

void SuperJump::OnActivate(CPlayerPed* player) {
    m_wasStanding = player->bIsStanding;
    m_inJump = false;
}

void SuperJump::Update(CPlayerPed* player, float dt) {
    if (player->bInVehicle || !player->bApplyGravity) { // in a car, or flying: not our business
        m_wasStanding = false;
        m_inJump = false;
        return;
    }
    int lvl = Level() - 1;
    bool standing = player->bIsStanding;

    // take-off: was on the ground last frame, now airborne and going up
    if (m_wasStanding && !standing && player->m_vecMoveSpeed.z > kTakeOffMinZ && PayForUse()) {
        CVector& v = player->m_vecMoveSpeed;
        v.x *= m_forward[lvl];
        v.y *= m_forward[lvl];
        v.z = m_jump[lvl];
        FallGuard::Protect(player);
        m_inJump = true;
        m_airTime = 0.0f;

        CVector feet = player->GetPosition();
        feet.z -= 1.0f;
        Fx::Spawn("wallbust", feet);
        Sound::Play(Sound::Whoosh, 0.6f + 0.2f * lvl);
    }

    if (m_inJump) {
        m_airTime += dt;
        if (standing && m_airTime > 0.2f) {
            // landing: dust, and at the top level a ground pound that throws everything around
            CVector feet = player->GetPosition();
            feet.z -= 1.0f;
            Fx::Spawn("wallbust", feet);
            if (m_poundRadius[lvl] > 0.0f) {
                Shockwave::Params params;
                params.radius = m_poundRadius[lvl];
                params.speed = m_poundSpeed[lvl];
                params.lift = 0.6f;
                Shockwave::Blast(feet, params, player);
                Fx::Spawn("heli_dust", feet);
                Sound::Play(Sound::Thunder, 0.5f);
            }
            m_inJump = false;
        }
    }
    m_wasStanding = standing;
}
