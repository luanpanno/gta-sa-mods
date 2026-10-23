#pragma once

#include "core/Power.h"

// Ammo refill: Left/Right choose how much and for which weapons, Enter gives it.
//
// How: CPed::GrantAmmo (adds, like an ammo pickup) / CPed::SetAmmo (sets), for the current weapon
// or every gun CJ carries (his 13 weapon slots, CPed::m_aWeapons). Melee weapons are skipped.
class AddAmmo : public Power {
public:
    const char* Name() const override { return "Add Ammo"; }
    const char* Id() const override { return "AddAmmo"; }
    const char* Description() const override { return "Left/Right choose the amount, Enter refills the current weapon or all of CJ's guns"; }
    const char* Category() const override { return "Spawn"; }
    Mode GetMode() const override { return Mode::OneShot; }

    bool HasPicker() const override { return true; }
    const char* PickerValue() const override;
    void PickerStep(int delta) override;

    void OnActivate(CPlayerPed* player) override;

private:
    int m_index = 1;
};
