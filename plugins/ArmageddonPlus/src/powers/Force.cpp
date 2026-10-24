#include "powers/Force.h"

#include "core/Config.h"
#include "core/Fx.h"
#include "core/Sound.h"
#include "core/Input.h"
#include "core/Log.h"
#include "core/Shockwave.h"

#include "plugin.h"
#include "CCamera.h"
#include "CObject.h"
#include "CPad.h"
#include "CPlayerPed.h"
#include "CPools.h"
#include "CVehicle.h"

#include <cstdio>

namespace {

const float kStepsPerSecond = 50.0f; // m_vecMoveSpeed is per 1/50 s step

} // namespace

void Force::LoadConfig(const Config& config, const char* section) {
    char key[32];
    for (int i = 0; i < kLevels; ++i) {
        std::snprintf(key, sizeof(key), "Radius%d", i + 1);
        m_radius[i] = config.GetFloat(section, key, m_radius[i]);
        std::snprintf(key, sizeof(key), "PushSpeed%d", i + 1);
        m_pushSpeed[i] = config.GetFloat(section, key, m_pushSpeed[i]);
        std::snprintf(key, sizeof(key), "PullSpeed%d", i + 1);
        m_pullSpeed[i] = config.GetFloat(section, key, m_pullSpeed[i]);
    }
    m_cooldown = config.GetFloat(section, "Cooldown", m_cooldown);
}

void Force::OnActivate(CPlayerPed*) {
    if (CPad* pad = CPad::GetPad(0))
        m_hadFireDisabled = pad->bDisablePlayerFireWeapon;
    m_cooldownLeft = 0.0f;
}

void Force::Blast(CPlayerPed* player, bool pull) {
    int lvl = Level() - 1;
    float radius = m_radius[lvl];
    CVector center = player->GetPosition();
    CVector forward = TheCamera.m_aCams[TheCamera.m_nActiveCam].m_vecFront;
    forward.z = 0.0f;
    forward.Normalize();

    Shockwave::Params params;
    params.radius = radius;
    params.pull = pull;
    params.cone = &forward; // only what's in front of CJ
    if (pull) {
        params.speed = m_pullSpeed[lvl];
        params.lift = 0.25f;
    } else {
        params.speed = m_pushSpeed[lvl];
        params.lift = 0.35f;
    }
    int count = Shockwave::Blast(center, params, player);
    // feedback: a dust shockwave in front of CJ + whoosh (outward for push, inward for pull)
    CVector ground;
    CVector front = center + forward * (pull ? radius * 0.5f : 3.0f);
    if (Fx::GroundBelow(front, 4.0f, ground))
        Fx::Spawn("wallbust", ground);
    Sound::Play(pull ? Sound::WhooshIn : Sound::Whoosh, 0.6f + 0.2f * lvl);

    m_cooldownLeft = m_cooldown;
    Log::Info("Force %s hit %d things", pull ? "pull" : "push", count);
}

void Force::Update(CPlayerPed* player, float dt) {
    if (m_cooldownLeft > 0.0f)
        m_cooldownLeft -= dt;
    if (player->bInVehicle)
        return;

    if (CPad* pad = CPad::GetPad(0))
        pad->bDisablePlayerFireWeapon = 1; // mouse buttons belong to the power while it's on

    if (m_cooldownLeft > 0.0f)
        return;
    if (Input::JustPressed(VK_LBUTTON) && PayForUse())
        Blast(player, false);
    else if (Input::JustPressed(VK_RBUTTON) && PayForUse())
        Blast(player, true);
}

void Force::OnDeactivate(CPlayerPed*) {
    if (CPad* pad = CPad::GetPad(0))
        pad->bDisablePlayerFireWeapon = m_hadFireDisabled;
}
