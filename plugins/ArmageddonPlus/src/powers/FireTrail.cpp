#include "powers/FireTrail.h"

#include "core/Config.h"

#include "plugin.h"
#include "CFireManager.h"
#include "CPlayerPed.h"
#include "CPools.h"
#include "CVehicle.h"

#include <cstdio>

void FireTrail::LoadConfig(const Config& config, const char* section) {
    char key[32];
    for (int i = 0; i < kLevels; ++i) {
        std::snprintf(key, sizeof(key), "Duration%d", i + 1);
        m_duration[i] = config.GetFloat(section, key, m_duration[i]);
        std::snprintf(key, sizeof(key), "Spread%d", i + 1);
        m_spread[i] = config.GetInt(section, key, m_spread[i]);
    }
    m_spacing = config.GetFloat(section, "Spacing", m_spacing);
}

void FireTrail::OnActivate(CPlayerPed* player) {
    m_wasFireProof = player->bFireProof;
    m_hasLastFire = false;
    m_vehicle = nullptr;
}

void FireTrail::SetVehicleFireproof(CVehicle* vehicle) {
    if (vehicle == m_vehicle)
        return;
    // switched vehicles: put the old one back (if it still exists)
    if (m_vehicle && CPools::ms_pVehiclePool->IsObjectValid(m_vehicle))
        m_vehicle->bFireProof = m_vehicleWasFireProof;
    m_vehicle = vehicle;
    if (vehicle) {
        m_vehicleWasFireProof = vehicle->bFireProof;
        vehicle->bFireProof = true;
    }
}

void FireTrail::Update(CPlayerPed* player, float) {
    player->bFireProof = true; // re-applied every frame, the game can reset it

    CVector firePos;
    if (player->bInVehicle && player->m_pVehicle) {
        CVehicle* vehicle = player->m_pVehicle;
        SetVehicleFireproof(vehicle);
        // a bit behind the car, at road level
        firePos = vehicle->GetPosition() - vehicle->GetForward() * 2.5f;
        firePos.z -= 0.5f;
    } else {
        SetVehicleFireproof(nullptr);
        if (!player->bIsStanding) {
            m_hasLastFire = false; // airborne (jumping/flying): no flames floating in the air
            return;
        }
        firePos = player->GetPosition();
        firePos.z -= 1.0f; // ped position is at the waist; flames go at the feet
    }

    if (m_hasLastFire && (firePos - m_lastFire).Magnitude() < m_spacing)
        return;

    int lvl = Level() - 1;
    unsigned int durationMs = (unsigned int)(m_duration[lvl] * 1000.0f);
    gFireManager.StartFire(firePos, 1.0f, 1, player, durationMs, (signed char)m_spread[lvl], 1);
    m_lastFire = firePos;
    m_hasLastFire = true;
}

void FireTrail::OnDeactivate(CPlayerPed* player) {
    player->bFireProof = m_wasFireProof;
    SetVehicleFireproof(nullptr);
}
