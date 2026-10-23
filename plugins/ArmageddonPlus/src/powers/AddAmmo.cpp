#include "powers/AddAmmo.h"

#include "plugin.h"
#include "CMessages.h"
#include "CPlayerPed.h"

#include <algorithm>

namespace {

struct Option {
    const char* label;
    unsigned int amount;
    bool allWeapons;
    bool setToAmount; // false = add, true = fill up to the amount
};

const Option kOptions[] = {
    {"Current +100", 100, false, false},
    {"Current +500", 500, false, false},
    {"All guns +500", 500, true, false},
    {"All guns +1000", 1000, true, false},
    {"All guns MAX", 9999, true, true},
};
const int kCount = sizeof(kOptions) / sizeof(kOptions[0]);

bool UsesAmmo(eWeaponType type) {
    // 0-15 fists/melee, 44-46 goggles + parachute: no ammo
    return type >= WEAPONTYPE_GRENADE && type < (eWeaponType)44;
}

} // namespace

const char* AddAmmo::PickerValue() const {
    return kOptions[m_index].label;
}

void AddAmmo::PickerStep(int delta) {
    m_index = std::clamp(m_index + (delta > 0 ? 1 : -1), 0, kCount - 1);
}

void AddAmmo::OnActivate(CPlayerPed* player) {
    const Option& option = kOptions[m_index];
    int refilled = 0;
    for (int slot = 0; slot < 13; ++slot) {
        if (!option.allWeapons && slot != player->m_nSelectedWepSlot)
            continue;
        eWeaponType type = player->m_aWeapons[slot].m_eWeaponType;
        if (!UsesAmmo(type))
            continue;
        if (option.setToAmount)
            player->SetAmmo(type, option.amount);
        else
            player->GrantAmmo(type, option.amount);
        ++refilled;
    }
    if (refilled == 0)
        CMessages::AddMessageJumpQ(option.allWeapons ? "~r~No guns to refill" : "~r~Current weapon doesn't use ammo", 1500, 0);
    else
        CMessages::AddMessageJumpQ("Ammo refilled", 1200, 0);
}
