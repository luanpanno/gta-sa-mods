#pragma once

#include "core/Power.h"

// Weapon picker: Left/Right choose any weapon in the game (Shift = 10 at a time), Enter gives it
// to CJ with ammo and makes it his current weapon.
//
// How: the weapon's model is streamed in (CStreaming), then CPed::GiveWeapon + SetCurrentWeapon,
// the same functions the game uses for pickups.
class GiveWeapon : public Power {
public:
    const char* Name() const override { return "Give Weapon"; }
    const char* Id() const override { return "GiveWeapon"; }
    const char* Description() const override { return "Left/Right choose a weapon (Shift: 10 at a time), Enter gives it to CJ with ammo"; }
    const char* Category() const override { return "Spawn"; }
    Mode GetMode() const override { return Mode::OneShot; }

    bool HasPicker() const override { return true; }
    const char* PickerValue() const override;
    void PickerStep(int delta) override;

    void LoadConfig(const Config& config, const char* section) override;
    void OnActivate(CPlayerPed* player) override;

private:
    int m_index = 0;
    int m_ammo = 500; // per gun; melee weapons get 1
};
