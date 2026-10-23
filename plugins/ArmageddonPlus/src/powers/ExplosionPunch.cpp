#include "powers/ExplosionPunch.h"

#include "core/Config.h"
#include "core/Log.h"

#include "plugin.h"
#include "CExplosion.h"
#include "CPlayerPed.h"
#include "CPools.h"
#include "CVehicle.h"

namespace {

struct LevelInfo {
    eExplosionType type;
    float camShake;
};

const LevelInfo kLevelInfo[ExplosionPunch::kLevels] = {
    {EXPLOSION_GRENADE, 0.0f},
    {EXPLOSION_ROCKET, 0.0f},
    {EXPLOSION_CAR, 0.6f},
};

bool IsMelee(unsigned int weaponType) {
    return weaponType < WEAPONTYPE_GRENADE; // 0 = fists ... 15 = cane
}

} // namespace

void ExplosionPunch::LoadConfig(const Config& config, const char* section) {
    m_cooldown = config.GetFloat(section, "Cooldown", m_cooldown);
}

void ExplosionPunch::OnActivate(CPlayerPed* player) {
    m_wasExplosionProof = player->bExplosionProof;
    m_lastHealth.clear();
    m_cooldownLeft = 0.0f;
}

void ExplosionPunch::Explode(CPlayerPed* player, CEntity* victim) {
    const LevelInfo& info = kLevelInfo[Level() - 1];
    CExplosion::AddExplosion(victim, player, info.type, victim->GetPosition(), 0, true, info.camShake, true);
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
