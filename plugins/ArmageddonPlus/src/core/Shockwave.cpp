#include "core/Shockwave.h"

#include "plugin.h"
#include "CObject.h"
#include "CPed.h"
#include "CPools.h"
#include "CVehicle.h"

namespace {
const float kStepsPerSecond = 50.0f; // m_vecMoveSpeed is per 1/50 s step
}

namespace Shockwave {

int Blast(const CVector& center, const Params& p, CPed* ignore) {
    CVehicle* ignoreVehicle = (ignore && ignore->bInVehicle) ? ignore->m_pVehicle : nullptr;
    int count = 0;

    auto affect = [&](CPhysical* entity) {
        CVector offset = entity->GetPosition() - center;
        float distance = offset.Magnitude();
        if (distance > p.radius || distance < 0.3f)
            return;
        CVector away = offset / distance;
        if (p.cone) {
            CVector flat(away.x, away.y, 0.0f);
            if (flat.Magnitude() > 0.01f && flat.Normalized().Dot(*p.cone) < p.coneDot)
                return;
        }

        float t = distance / p.radius; // 0 center .. 1 edge
        float strength;
        CVector velocity;
        if (p.pull) {
            // slower when already close, so things land near the center instead of on it
            strength = p.speed * (p.edgeFactor + (1.0f - p.edgeFactor) * t);
            velocity = away * -strength;
        } else {
            strength = p.speed * (1.0f - (1.0f - p.edgeFactor) * t);
            velocity = away * strength;
        }
        velocity.z = strength * p.lift;

        entity->SetIsStatic(false);
        if (entity->m_nType == ENTITY_TYPE_PED) {
            CVector pos = entity->GetPosition();
            entity->SetPosn(pos.x, pos.y, pos.z + 0.3f);
        }
        entity->m_vecMoveSpeed = velocity / kStepsPerSecond;
        ++count;
    };

    auto* peds = CPools::ms_pPedPool;
    for (int i = 0; i < peds->m_nSize; ++i) {
        CPed* ped = peds->GetAt(i);
        if (ped && ped != ignore && !ped->bInVehicle)
            affect(ped);
    }
    auto* vehicles = CPools::ms_pVehiclePool;
    for (int i = 0; i < vehicles->m_nSize; ++i) {
        CVehicle* vehicle = vehicles->GetAt(i);
        if (vehicle && vehicle != ignoreVehicle)
            affect(vehicle);
    }
    auto* objects = CPools::ms_pObjectPool;
    for (int i = 0; i < objects->m_nSize; ++i)
        if (CObject* object = objects->GetAt(i))
            affect(object);
    return count;
}

} // namespace Shockwave
