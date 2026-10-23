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
#include "CText.h"
#include "CVehicleModelInfo.h"
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

void SpawnVehicle::LoadConfig(const Config& config, const char* section) {
    m_startModel = config.GetInt(section, "Model", 411);
}

void SpawnVehicle::BuildList() const {
    if (!m_models.empty())
        return; // built on first use: model info and texts only exist once the game has loaded
    for (int id = 400; id <= 611; ++id) {
        auto info = CModelInfo::GetModelInfo(id);
        if (!info || info->GetModelType() != MODEL_INFO_VEHICLE)
            continue;
        auto vehicleInfo = reinterpret_cast<CVehicleModelInfo*>(info);
        switch (vehicleInfo->m_nVehicleType) {
        case VEHICLE_TRAIN: case VEHICLE_FHELI: case VEHICLE_FPLANE:
            continue; // trains need tracks; RC toys can't be driven by CJ
        default:
            break;
        }
        // display name: the game's text for the model's GXT key (e.g. "INFERNU" -> "Infernus")
        const char* text = TheText.Get(vehicleInfo->m_szGameName);
        std::string name = (text && *text) ? text : vehicleInfo->m_szGameName;
        m_models.push_back(id);
        m_names.push_back(name);
        if (id == m_startModel)
            m_index = (int)m_models.size() - 1;
    }
    Log::Info("SpawnVehicle: %d spawnable vehicles", (int)m_models.size());
}

const char* SpawnVehicle::PickerValue() const {
    BuildList();
    if (m_models.empty())
        return "-";
    static char text[64];
    std::snprintf(text, sizeof(text), "%s  %d", m_names[m_index].c_str(), m_models[m_index]);
    return text;
}

void SpawnVehicle::PickerStep(int delta) {
    BuildList();
    int count = (int)m_models.size();
    if (count == 0)
        return;
    m_index = ((m_index + delta) % count + count) % count; // wrap around both ways
}

void SpawnVehicle::OnActivate(CPlayerPed* player) {
    static char message[96]; // static: CMessages keeps the pointer while the text is on screen

    if (player->m_nAreaCode != 0) {
        CMessages::AddMessageJumpQ("~r~Can't spawn vehicles inside interiors", 3000, 0);
        return;
    }
    BuildList();
    if (m_models.empty())
        return;
    int model = m_models[m_index];

    // 6 m in front of CJ (x = right, y = forward, z = up, relative to CJ); bigger for planes/boats
    CVector position = player->TransformFromObjectSpace(CVector(0.0f, 8.0f, 0.0f));
    CVehicle* vehicle = CreateVehicle(model, position, player->m_fHeadingCurrent);

    if (!vehicle) {
        std::snprintf(message, sizeof(message), "~r~Failed to spawn %s", m_names[m_index].c_str());
        Log::Info("SpawnVehicle: failed for model %d", model);
    } else {
        std::snprintf(message, sizeof(message), "Spawned ~y~%s", m_names[m_index].c_str());
    }
    CMessages::AddMessageJumpQ(message, 2000, 0);
}