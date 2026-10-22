#include "powers/Telekinesis.h"

#include "core/Config.h"
#include "core/Input.h"
#include "core/Log.h"

#include "plugin.h"
#include "CCamera.h"
#include "CColPoint.h"
#include "CObject.h"
#include "CPad.h"
#include "CPlayerPed.h"
#include "CPools.h"
#include "CStreaming.h"
#include "CVehicle.h"
#include "CWeaponInfo.h"
#include "CWorld.h"

#include <cstdio>

namespace {

const float kStepsPerSecond = 50.0f; // m_vecMoveSpeed is per 1/50 s step
const float kHoldStiffness = 8.0f;   // how hard the held thing is pulled to the hold point (1/s)
const float kHoldMaxSpeed = 40.0f;   // m/s

bool IsMovable(CEntity* entity) {
    return entity && (entity->m_nType == ENTITY_TYPE_VEHICLE || entity->m_nType == ENTITY_TYPE_PED ||
                      entity->m_nType == ENTITY_TYPE_OBJECT);
}

void LoadModel(int modelIndex) {
    CStreaming::RequestModel(modelIndex, GAME_REQUIRED);
    CStreaming::LoadAllRequestedModels(false);
}

} // namespace

void Telekinesis::LoadConfig(const Config& config, const char* section) {
    char key[32];
    for (int i = 0; i < kLevels; ++i) {
        std::snprintf(key, sizeof(key), "ThrowSpeed%d", i + 1);
        m_throwSpeed[i] = config.GetFloat(section, key, m_throwSpeed[i]);
    }
    m_range = config.GetFloat(section, "Range", m_range);
    m_aimWeapon = (eWeaponType)config.GetInt(section, "AimWeapon", WEAPONTYPE_DESERT_EAGLE);
}

// --- the aiming pistol ---------------------------------------------------------------------

void Telekinesis::EquipAimWeapon(CPlayerPed* player) {
    CWeaponInfo* info = CWeaponInfo::GetWeaponInfo(m_aimWeapon, 1);
    if (!info)
        return;
    m_slot = info->m_nSlot;
    CWeapon& current = player->m_aWeapons[m_slot];
    m_savedType = current.m_eWeaponType;
    m_savedAmmo = current.m_nAmmoTotal;
    m_savedClip = current.m_nAmmoInClip;
    m_savedCurrent = player->m_aWeapons[player->m_nSelectedWepSlot].m_eWeaponType;

    LoadModel(info->m_nModelId); // giving a weapon whose model isn't loaded = invisible gun
    player->GiveWeapon(m_aimWeapon, 1, true);
    player->SetCurrentWeapon(m_aimWeapon);
    m_equipped = true;
}

void Telekinesis::RestoreWeapons(CPlayerPed* player) {
    if (!m_equipped)
        return;
    m_equipped = false;
    if (player->m_fHealth <= 0.0f)
        return; // dying: the game takes all weapons anyway

    player->ClearWeapon(m_aimWeapon);
    if (m_savedType != WEAPONTYPE_UNARMED) {
        if (CWeaponInfo* info = CWeaponInfo::GetWeaponInfo(m_savedType, 1))
            LoadModel(info->m_nModelId);
        player->GiveWeapon(m_savedType, m_savedAmmo, true);
        player->m_aWeapons[m_slot].m_nAmmoInClip = m_savedClip;
    }
    player->SetCurrentWeapon(m_savedCurrent);
}

// --- grab / hold / throw -------------------------------------------------------------------

void Telekinesis::AimRay(CPlayerPed* player, CVector& from, CVector& direction) const {
    const CCam& cam = TheCamera.m_aCams[TheCamera.m_nActiveCam];
    if (Input::IsDown(VK_RBUTTON)) {
        // aiming the pistol: the crosshair is off-center, the game knows exactly where it points
        CVector target;
        TheCamera.Find3rdPersonCamTargetVector(m_range, player->GetPosition(), &from, &target);
        direction = (target - from).Normalized();
    } else {
        from = cam.m_vecSource;
        direction = cam.m_vecFront;
    }
}

CPhysical* Telekinesis::FindTarget(CPlayerPed* player) {
    CVector from, direction;
    AimRay(player, from, direction);
    CVector to = from + direction * m_range;

    CColPoint hit;
    CEntity* entity = nullptr;
    CWorld::pIgnoreEntity = player; // the ray starts behind CJ; don't grab ourselves
    bool found = CWorld::ProcessLineOfSight(from, to, hit, entity, true, true, true, true, false, false, false, false);
    CWorld::pIgnoreEntity = nullptr;

    if (!found || !IsMovable(entity))
        return nullptr; // nothing, or a building
    if (entity->m_nType == ENTITY_TYPE_PED && static_cast<CPed*>(entity)->bInVehicle)
        return static_cast<CPed*>(entity)->m_pVehicle; // a driver -> take the whole car
    return static_cast<CPhysical*>(entity);
}

bool Telekinesis::HeldStillExists() const {
    // the game may delete it at any time (despawn, explosion clean-up); a stale pointer = crash
    switch (m_heldType) {
    case ENTITY_TYPE_VEHICLE: return CPools::ms_pVehiclePool->IsObjectValid(static_cast<CVehicle*>(m_held));
    case ENTITY_TYPE_PED:     return CPools::ms_pPedPool->IsObjectValid(static_cast<CPed*>(m_held));
    case ENTITY_TYPE_OBJECT:  return CPools::ms_pObjectPool->IsObjectValid(static_cast<CObject*>(m_held));
    }
    return false;
}

void Telekinesis::Grab(CPlayerPed* player, CPhysical* entity) {
    CVector from, direction;
    AimRay(player, from, direction);

    m_held = entity;
    m_heldType = entity->m_nType;
    m_heldHadGravity = entity->bApplyGravity;
    m_holdDistance = (entity->GetPosition() - from).Magnitude();
    if (m_holdDistance < 6.0f)
        m_holdDistance = 6.0f;
    if (m_holdDistance > 25.0f)
        m_holdDistance = 25.0f;

    entity->SetIsStatic(false); // parked cars / placed objects are asleep until woken
    entity->bApplyGravity = false;
    if (m_heldType == ENTITY_TYPE_PED) {
        // lift peds off the ground so physics (not their walk animation) moves them
        CVector pos = entity->GetPosition();
        entity->SetPosn(pos.x, pos.y, pos.z + 0.3f);
    }
    Log::Info("Telekinesis: grabbed entity type %d", m_heldType);
}

void Telekinesis::Hold(CPlayerPed* player) {
    CVector from, direction;
    AimRay(player, from, direction);
    CVector holdPoint = from + direction * m_holdDistance;

    CVector velocity = (holdPoint - m_held->GetPosition()) * kHoldStiffness; // m/s, stronger when further
    if (velocity.Magnitude() > kHoldMaxSpeed)
        velocity = velocity.Normalized() * kHoldMaxSpeed;

    m_held->bApplyGravity = false;
    m_held->m_vecMoveSpeed = velocity / kStepsPerSecond;
    m_held->m_vecTurnSpeed *= 0.85f; // calm the spinning so it doesn't tumble wildly
}

void Telekinesis::Release(CPlayerPed* player, bool throwIt) {
    if (m_held && HeldStillExists()) {
        m_held->bApplyGravity = m_heldHadGravity;
        if (throwIt) {
            CVector from, direction;
            AimRay(player, from, direction);
            m_held->m_vecMoveSpeed = direction * (m_throwSpeed[Level() - 1] / kStepsPerSecond);
        }
    }
    m_held = nullptr;
}

// --- power hooks ---------------------------------------------------------------------------

void Telekinesis::OnActivate(CPlayerPed* player) {
    if (CPad* pad = CPad::GetPad(0))
        m_hadFireDisabled = pad->bDisablePlayerFireWeapon;
    m_held = nullptr;
    if (!player->bInVehicle)
        EquipAimWeapon(player);
}

void Telekinesis::Update(CPlayerPed* player, float) {
    if (player->bInVehicle) {
        Release(player, false);
        return;
    }
    if (!m_equipped)
        EquipAimWeapon(player); // turned on while driving, now on foot
    else if (player->m_aWeapons[player->m_nSelectedWepSlot].m_eWeaponType != m_aimWeapon)
        player->SetCurrentWeapon(m_aimWeapon); // no switching weapons while it's on

    if (CPad* pad = CPad::GetPad(0))
        pad->bDisablePlayerFireWeapon = 1; // LMB grabs instead of shooting

    if (m_held) {
        if (!HeldStillExists())
            m_held = nullptr; // it got deleted
        else if (!Input::IsDown(VK_LBUTTON))
            Release(player, true); // let go of LMB = throw
        else
            Hold(player);
        return;
    }

    if (Input::JustPressed(VK_LBUTTON) && Input::IsDown(VK_RBUTTON))
        if (CPhysical* target = FindTarget(player))
            Grab(player, target);
}

void Telekinesis::OnDeactivate(CPlayerPed* player) {
    Release(player, false); // drop it where it is
    if (CPad* pad = CPad::GetPad(0))
        pad->bDisablePlayerFireWeapon = m_hadFireDisabled;
    RestoreWeapons(player);
}
