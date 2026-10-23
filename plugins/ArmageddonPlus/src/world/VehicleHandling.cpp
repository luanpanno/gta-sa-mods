#include "world/VehicleHandling.h"

#include "core/Config.h"
#include "core/Fx.h"
#include "core/Input.h"
#include "core/Sound.h"

#include "plugin.h"
#include "CAutomobile.h"
#include "CPlayerPed.h"
#include "CPools.h"
#include "CVehicle.h"

#include <algorithm>
#include <cstdio>

namespace {
const float kStepsPerSecond = 50.0f; // m_vecMoveSpeed is per 1/50 s step
}

const char* VehicleHandling::LevelName(int level) const {
    static const char* kNames[kLevels] = {"Sport", "Race", "Rocket"};
    return level >= 1 && level <= kLevels ? kNames[level - 1] : nullptr;
}

void VehicleHandling::LoadConfig(const Config& config, const char* section) {
    char key[32];
    for (int i = 0; i < kLevels; ++i) {
        std::snprintf(key, sizeof(key), "TurboAccel%d", i + 1);
        m_turboAccel[i] = config.GetFloat(section, key, m_turboAccel[i]);
        std::snprintf(key, sizeof(key), "TopSpeed%d", i + 1);
        m_topSpeed[i] = config.GetFloat(section, key, m_topSpeed[i]);
        std::snprintf(key, sizeof(key), "Grip%d", i + 1);
        m_grip[i] = config.GetFloat(section, key, m_grip[i]);
        std::snprintf(key, sizeof(key), "Jump%d", i + 1);
        m_jump[i] = config.GetFloat(section, key, m_jump[i]);
    }
    m_turboKey = config.GetKey(section, "TurboKey", VK_SHIFT);
    m_jumpKey = config.GetKey(section, "JumpKey", 'J');
}

void VehicleHandling::TrackVehicle(CVehicle* vehicle) {
    if (vehicle == m_vehicle)
        return;
    if (m_vehicle && CPools::ms_pVehiclePool->IsObjectValid(m_vehicle))
        m_vehicle->bTyresDontBurst = m_vehicleTyresWereProof;
    m_vehicle = vehicle;
    m_upsideDownFor = 0.0f;
    if (vehicle) {
        m_vehicleTyresWereProof = vehicle->bTyresDontBurst;
        vehicle->bTyresDontBurst = true;
    }
}

void VehicleHandling::Update(CPlayerPed* player, float dt) {
    CVehicle* vehicle = (player->bInVehicle && player->m_pVehicle && player->m_pVehicle->m_pDriver == player)
                            ? player->m_pVehicle : nullptr; // only when CJ is the driver
    TrackVehicle(vehicle);
    if (!vehicle)
        return;

    int lvl = Level() - 1;
    CVector forward = vehicle->GetForward();
    CVector right = vehicle->GetRight();
    CVector up = vehicle->GetUp();
    CVector velocity = vehicle->m_vecMoveSpeed * kStepsPerSecond; // m/s
    CVector ground;
    bool onGround = Fx::GroundBelow(vehicle->GetPosition(), 2.5f, ground);

    // --- turbo -------------------------------------------------------------------------
    bool turbo = Input::IsDown(m_turboKey);
    if (turbo && !m_turboWasDown)
        Sound::Play(Sound::Whoosh, 0.5f);
    m_turboWasDown = turbo;
    if (turbo && velocity.Dot(forward) < m_topSpeed[lvl])
        velocity += forward * (m_turboAccel[lvl] * dt);

    // --- grip: remove part of the sideways slide (cars on the ground only) --------------
    if (onGround && vehicle->m_nVehicleClass == VEHICLE_AUTOMOBILE) {
        float sideways = velocity.Dot(right);
        velocity -= right * (sideways * std::min(1.0f, m_grip[lvl] * dt));
    }

    // --- jump ---------------------------------------------------------------------------
    if (Input::JustPressed(m_jumpKey) && onGround && up.z > 0.5f) {
        velocity.z += m_jump[lvl];
        Sound::Play(Sound::Whoosh, 0.7f);
        Fx::Spawn("wallbust", ground);
    }

    vehicle->m_vecMoveSpeed = velocity / kStepsPerSecond;

    // --- auto-flip: upside down and (almost) stopped for a second -> back on the wheels --
    if (up.z < -0.2f && velocity.Magnitude() < 2.0f) {
        m_upsideDownFor += dt;
        if (m_upsideDownFor > 1.0f) {
            CVector pos = vehicle->GetPosition();
            float heading = vehicle->GetHeading();
            vehicle->SetOrientation(0.0f, 0.0f, heading);
            vehicle->SetPosn(pos.x, pos.y, pos.z + 1.5f);
            vehicle->m_vecTurnSpeed = CVector(0.0f, 0.0f, 0.0f);
            m_upsideDownFor = 0.0f;
        }
    } else {
        m_upsideDownFor = 0.0f;
    }
}

void VehicleHandling::OnDeactivate(CPlayerPed*) {
    TrackVehicle(nullptr);
}
