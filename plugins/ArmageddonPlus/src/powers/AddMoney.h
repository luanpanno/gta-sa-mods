#pragma once

#include "core/Power.h"

// Money adder: Left/Right choose an amount ($1,000 ... $10,000,000), Enter adds it.
//
// How: CPlayerInfo::m_nMoney is CJ's real money; the HUD counter (m_nDisplayMoney) then rolls
// up to it on its own, like after a mission reward.
class AddMoney : public Power {
public:
    const char* Name() const override { return "Add Money"; }
    const char* Id() const override { return "AddMoney"; }
    const char* Description() const override { return "Left/Right choose an amount, Enter adds it to CJ's money"; }
    const char* Category() const override { return "Spawn"; }
    Mode GetMode() const override { return Mode::OneShot; }

    bool HasPicker() const override { return true; }
    const char* PickerValue() const override;
    void PickerStep(int delta) override;

    void OnActivate(CPlayerPed* player) override;

private:
    int m_index = 2; // $100,000
};
