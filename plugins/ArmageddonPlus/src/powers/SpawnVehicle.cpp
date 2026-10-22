#include "powers/SpawnVehicle.h"

#include "core/Config.h"
#include "core/Log.h"

#include "plugin.h"
#include "CBmx.h"
#include "CBoat.h"
#include "CHeli.h"
#include "CMessages.h"
#include "CModelInfo.h"
#include "CMonsterTruck.h"
#include "CPlane.h"
#include "CPlayerPed.h"
#include "CQuadBike.h"
#include "CStreaming.h"
#include "CTheScripts.h"
#include "CTrailer.h"
#include "CTrain.h"
#include "CWorld.h"

#include <cstdio>

using namespace plugin;

namespace {

// Adapted from plugin-sdk's examples/CreateCar (GTA Community, https://github.com/DK22Pac/plugin-sdk).
// Same steps as the CLEO version (request model -> load -> create -> place), but done directly:
// the vehicle class must match the model's type, CWorld::Add makes it exist in the world.
CVehicle* CreateVehicle(int modelIndex, CVector position, float heading) {
    unsigned char oldFlags = CStreaming::ms_aInfoForModel[modelIndex].m_nFlags;

    CStreaming::RequestModel(modelIndex, GAME_REQUIRED);
    CStreaming::LoadAllRequestedModels(false);
    if (CStreaming::ms_aInfoForModel[modelIndex].m_nLoadState != LOADSTATE_LOADED)
        return nullptr;

    // like CLEO's mark_model_as_no_longer_needed: let the streamer unload it later
    if (!(oldFlags & GAME_REQUIRED)) {
        CStreaming::SetModelIsDeletable(modelIndex);
        CStreaming::SetModelTxdIsDeletable(modelIndex);
    }

    auto modelInfo = CModelInfo::GetModelInfo(modelIndex);
    if (!modelInfo || modelInfo->GetModelType() != MODEL_INFO_VEHICLE)
        return nullptr;
    auto vehModelInfo = reinterpret_cast<CVehicleModelInfo*>(modelInfo);

    CVehicle* vehicle = nullptr;
    switch (vehModelInfo->m_nVehicleType) {
    case VEHICLE_BMX:        vehicle = new CBmx(modelIndex, eVehicleCreatedBy::RANDOM_VEHICLE); break;
    case VEHICLE_BIKE:       vehicle = new CBike(modelIndex, eVehicleCreatedBy::RANDOM_VEHICLE); break;
    case VEHICLE_QUAD:       vehicle = new CQuadBike(modelIndex, eVehicleCreatedBy::RANDOM_VEHICLE); break;
    case VEHICLE_AUTOMOBILE: vehicle = new CAutomobile(modelIndex, eVehicleCreatedBy::RANDOM_VEHICLE, true); break;
    case VEHICLE_MTRUCK:     vehicle = new CMonsterTruck(modelIndex, eVehicleCreatedBy::RANDOM_VEHICLE); break;
    case VEHICLE_TRAILER:    vehicle = new CTrailer(modelIndex, eVehicleCreatedBy::RANDOM_VEHICLE); break;
    case VEHICLE_BOAT:       vehicle = new CBoat(modelIndex, eVehicleCreatedBy::RANDOM_VEHICLE); break;
    case VEHICLE_HELI:       vehicle = new CHeli(modelIndex, eVehicleCreatedBy::RANDOM_VEHICLE); break;
    case VEHICLE_PLANE:      vehicle = new CPlane(modelIndex, eVehicleCreatedBy::RANDOM_VEHICLE); break;
    default: break; // trains need tracks; not supported here
    }
    if (!vehicle)
        return nullptr;

    position.z += vehicle->GetHeightAboveRoad();
    vehicle->SetOrientation(0.0f, 0.0f, heading);
    vehicle->Teleport(position); // includes CWorld::Add

    vehicle->m_nStatus = eEntityStatus::STATUS_ABANDONED;
    vehicle->bEngineOn = false;
    vehicle->m_eDoorLock = DOORLOCK_UNLOCKED;

    switch (vehicle->m_nVehicleClass) {
    case VEHICLE_BIKE:
        reinterpret_cast<CBike*>(vehicle)->bOnSideStand = true;
        reinterpret_cast<CBike*>(vehicle)->PlaceOnRoadProperly();
        break;
    case VEHICLE_AUTOMOBILE:
        reinterpret_cast<CAutomobile*>(vehicle)->PlaceOnRoadProperly();
        break;
    }
    vehicle->UpdateRwMatrix();
    CTheScripts::ClearSpaceForMissionEntity(vehicle->GetPosition(), vehicle);
    vehicle->ProcessControl();
    return vehicle;
}

} // namespace

int SpawnVehicle::DefaultKey() const {
    return VK_F11;
}

void SpawnVehicle::LoadConfig(const Config& config, const char* section) {
    m_model = config.GetInt(section, "Model", 411);
}

void SpawnVehicle::OnActivate(CPlayerPed* player) {
    static char message[96]; // static: CMessages keeps the pointer while the text is on screen

    if (player->m_nAreaCode != 0) {
        CMessages::AddMessageJumpQ("~r~Can't spawn vehicles inside interiors", 3000, 0);
        return;
    }
    if (m_model < 400 || m_model > 611) {
        std::snprintf(message, sizeof(message), "~r~Invalid vehicle model %d (400-611)", m_model);
        CMessages::AddMessageJumpQ(message, 3000, 0);
        return;
    }

    // 6 m in front of CJ (x = right, y = forward, z = up, relative to CJ)
    CVector position = player->TransformFromObjectSpace(CVector(0.0f, 6.0f, 0.0f));
    CVehicle* vehicle = CreateVehicle(m_model, position, player->m_fHeadingCurrent);

    if (!vehicle) {
        std::snprintf(message, sizeof(message), "~r~Failed to spawn model %d", m_model);
        Log::Info("SpawnVehicle: failed for model %d", m_model);
    } else {
        std::snprintf(message, sizeof(message), "Spawned vehicle ~y~%d", m_model);
    }
    CMessages::AddMessageJumpQ(message, 2000, 0);
}
