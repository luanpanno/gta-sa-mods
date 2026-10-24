#include "powers/ExplosionPunch.h"

#include "core/Config.h"
#include "core/Fx.h"
#include "core/Log.h"
#include "core/Shockwave.h"
#include "core/Sound.h"

#include "plugin.h"
#include "CExplosion.h"
#include "CPlayerPed.h"
#include "CPools.h"
#include "CVehicle.h"

#include <cmath>
#include <cstdio>

namespace {

const float kStepsPerSecond = 50.0f; // m_vecMoveSpeed is per 1/50 s step

// main blast at the victim, plus a ring of extra blasts around it
struct LevelInfo {
    eExplosionType main;
    eExplosionType ring;
    float camShake;
};
const LevelInfo kLevelInfo[ExplosionPunch::kLevels] = {
    {EXPLOSION_CAR, EXPLOSION_CAR, 0.6f},
    {EXPLOSION_AIRCRAFT, EXPLOSION_CAR, 1.1f},
    {EXPLOSION_AIRCRAFT, EXPLOSION_AIRCRAFT, 1.6f},
};

bool IsMelee(unsigned int weaponType) {
    return weaponType < WEAPONTYPE_GRENADE; // 0 = fists ... 15 = cane
}

} // namespace

void ExplosionPunch::LoadConfig(const Config& config, const char* section) {
    char key[32];
    for (int i = 0; i < kLevels; ++i) {
        std::snprintf(key, sizeof(key), "RingBlasts%d", i + 1);
        m_ringBlasts[i] = config.GetInt(section, key, m_ringBlasts[i]);
        std::snprintf(key, sizeof(key), "RingRadius%d", i + 1);
        m_ringRadius[i] = config.GetFloat(section, key, m_ringRadius[i]);
        std::snprintf(key, sizeof(key), "ShockRadius%d", i + 1);
        m_shockRadius[i] = config.GetFloat(section, key, m_shockRadius[i]);
        std::snprintf(key, sizeof(key), "ShockSpeed%d", i + 1);
        m_shockSpeed[i] = config.GetFloat(section, key, m_shockSpeed[i]);
        std::snprintf(key, sizeof(key), "Launch%d", i + 1);
        m_launch[i] = config.GetFloat(section, key, m_launch[i]);
    }
    m_cooldown = config.GetFloat(section, "Cooldown", m_cooldown);
}

void ExplosionPunch::OnActivate(CPlayerPed* player) {
    m_wasExplosionProof = player->bExplosionProof;
    m_lastHealth.clear();
    m_cooldownLeft = 0.0f;
}

void ExplosionPunch::Explode(CPlayerPed* player, CEntity* victim) {
    int lvl = Level() - 1;
    const LevelInfo& info = kLevelInfo[lvl];
    CVector at = victim->GetPosition();

    // 1) launch the victim away from CJ, like a cannon shot
    CVector away = at - player->GetPosition();
    away.z = 0.0f;
    if (away.Magnitude() < 0.1f)
        away = player->GetForward();
    away.Normalize();
    auto* physical = static_cast<CPhysical*>(victim);
    physical->SetIsStatic(false);
    if (victim->m_nType == ENTITY_TYPE_PED)
        victim->SetPosn(at.x, at.y, at.z + 0.4f); // off the ground so physics moves him
    physical->m_vecMoveSpeed = (away * m_launch[lvl] + CVector(0.0f, 0.0f, m_launch[lvl] * 0.45f)) / kStepsPerSecond;

    // 2) the main blast on the victim, and a ring of blasts around it
    CExplosion::AddExplosion(victim, player, info.main, at, 0, true, info.camShake, true);
    for (int i = 0; i < m_ringBlasts[lvl]; ++i) {
        float angle = 6.2832f * i / m_ringBlasts[lvl];
        CVector point = at + CVector(std::cos(angle), std::sin(angle), 0.0f) * m_ringRadius[lvl];
        CExplosion::AddExplosion(nullptr, player, info.ring, point, 50 * (i + 1), true, 0.0f, true); // staggered
    }

    // 3) shockwave: everything around gets thrown outward
    Shockwave::Params params;
    params.radius = m_shockRadius[lvl];
    params.speed = m_shockSpeed[lvl];
    params.lift = 0.45f;
    Shockwave::Blast(at, params, player);
    Fx::Spawn("wallbust", at);
    Sound::Play(Sound::Thunder, 0.5f + 0.25f * lvl);

    m_cooldownLeft = m_cooldown;
}
void ExplosionPunch::Update(CPlayerPed* player, float dt) {
    // our own blasts must not hurt CJ (re-applied every frame, the game can reset it)
    player->bExplosionProof = true;

    if (m_cooldownLeft > 0.0f)
        m_cooldownLeft -= dt;

    const CVector& center = player->GetPosition();
    std::unordered_map<CEntity*, float> health;
    CEntity* victim = nullptr;

    auto* peds = CPools::ms_pPedPool;
    for (int i = 0; i < peds->m_nSize; ++i) {
        CPed* ped = peds->GetAt(i);
        if (!ped || ped == player)
            continue;
        if ((ped->GetPosition() - center).Magnitude() > m_radius)
            continue;
        health[ped] = ped->m_fHealth;
        auto last = m_lastHealth.find(ped);
        if (last != m_lastHealth.end() && ped->m_fHealth < last->second &&
            ped->m_pLastEntityDamage == player && IsMelee(ped->m_nLastWeaponDamage))
            victim = ped;
    }

    auto* vehicles = CPools::ms_pVehiclePool;
    for (int i = 0; i < vehicles->m_nSize; ++i) {
        CVehicle* vehicle = vehicles->GetAt(i);
        if (!vehicle || (player->bInVehicle && vehicle == player->m_pVehicle))
            continue;
        if ((vehicle->GetPosition() - center).Magnitude() > m_radius)
            continue;
        health[vehicle] = vehicle->m_fHealth;
        auto last = m_lastHealth.find(vehicle);
        if (last != m_lastHealth.end() && vehicle->m_fHealth < last->second &&
            vehicle->m_pLastDamageEntity == player && IsMelee(vehicle->m_nLastWeaponDamageType))
            victim = vehicle;
    }

    m_lastHealth = std::move(health);

    if (victim && m_cooldownLeft <= 0.0f && PayForUse()) {
        Explode(player, victim);
        Log::Info("ExplosionPunch: boom (level %d)", Level());
    }
}

void ExplosionPunch::OnDeactivate(CPlayerPed* player) {
    player->bExplosionProof = m_wasExplosionProof;
    m_lastHealth.clear();
}
