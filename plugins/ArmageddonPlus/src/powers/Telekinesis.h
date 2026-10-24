#pragma once

#include "core/Power.h"

#include "eWeaponType.h"

class CEntity;
class CPhysical;
struct CVector;

// Phase 3.5: Telekinesis. 3 levels = throw strength.
// Turn it on in the menu; CJ draws a Desert Eagle (it can't fire) so you get a real crosshair:
//
//   hold RMB (aim) + click LMB   grab the car / person / object under the crosshair
//   keep LMB held                it floats in front of you and follows your aim
//   release LMB                  throw it where you're aiming
//
// How: a ray through the crosshair (CCamera::Find3rdPersonCamTargetVector + CWorld::ProcessLineOfSight)
// finds the target. The held thing gets gravity off and, every frame, a velocity pulling it toward a
// point along the aim, so the game's physics moves it (it still collides with the world).
// Your own weapon in the pistol slot is put back when the power turns off.
class Telekinesis : public Power {
public:
    static constexpr int kLevels = 3;

    Telekinesis() {
        AddSetting("Throw speed", "ThrowSpeed", m_throwSpeed, true, 10.0f, 250.0f, 5.0f, "m/s"); // Customize page
        AddSetting("Grab range", "Range", &m_range, false, 10.0f, 300.0f, 10.0f, "m");
    }

    const char* Name() const override { return "Telekinesis"; }
    const char* Id() const override { return "Telekinesis"; }
    const char* Description() const override { return "Aim (RMB) + click (LMB) to grab, hold to carry, release to throw"; }
    int MaxLevel() const override { return kLevels; }
    const char* ExclusiveGroup() const override { return "mouse"; }

    void LoadConfig(const Config& config, const char* section) override;
    void OnActivate(CPlayerPed* player) override;
    void Update(CPlayerPed* player, float dt) override;
    void OnDeactivate(CPlayerPed* player) override;

private:
    void AimRay(CPlayerPed* player, CVector& from, CVector& direction) const; // through the crosshair
    CPhysical* FindTarget(CPlayerPed* player);
    bool HeldStillExists() const;
    void Grab(CPlayerPed* player, CPhysical* entity);
    void Hold(CPlayerPed* player);
    void Release(CPlayerPed* player, bool throwIt);

    void EquipAimWeapon(CPlayerPed* player);
    void RestoreWeapons(CPlayerPed* player);

    float m_throwSpeed[kLevels] = {40.0f, 80.0f, 140.0f}; // m/s
    float m_range = 80.0f; // how far the crosshair can grab
    eWeaponType m_aimWeapon = WEAPONTYPE_DESERT_EAGLE;

    CPhysical* m_held = nullptr;
    int m_heldType = 0;           // eEntityType of m_held, to re-validate it against the right pool
    bool m_heldHadGravity = true;
    float m_holdDistance = 10.0f; // from the camera, set when grabbed
    char m_hadFireDisabled = 0;   // CPad::bDisablePlayerFireWeapon before we set it

    // what CJ had before we handed him the pistol
    bool m_equipped = false;
    int m_slot = 0;
    eWeaponType m_savedType = WEAPONTYPE_UNARMED;
    unsigned int m_savedAmmo = 0;
    unsigned int m_savedClip = 0;
    eWeaponType m_savedCurrent = WEAPONTYPE_UNARMED;
};
