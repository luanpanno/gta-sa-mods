#include "powers/GiveWeapon.h"

#include "core/Config.h"

#include "plugin.h"
#include "CMessages.h"
#include "CPlayerPed.h"
#include "CStreaming.h"
#include "CWeaponInfo.h"

#include <cstdio>

namespace {

struct WeaponEntry {
    eWeaponType type;
    const char* name;
};

// every weapon CJ can hold (the detonator comes with the satchel charges)
const WeaponEntry kWeapons[] = {
    {WEAPONTYPE_BRASSKNUCKLE, "Brass Knuckles"}, {WEAPONTYPE_GOLFCLUB, "Golf Club"},
    {WEAPONTYPE_NIGHTSTICK, "Nightstick"},       {WEAPONTYPE_KNIFE, "Knife"},
    {WEAPONTYPE_BASEBALLBAT, "Baseball Bat"},    {WEAPONTYPE_SHOVEL, "Shovel"},
    {WEAPONTYPE_POOLCUE, "Pool Cue"},            {WEAPONTYPE_KATANA, "Katana"},
    {WEAPONTYPE_CHAINSAW, "Chainsaw"},           {WEAPONTYPE_DILDO1, "Purple Dildo"},
    {WEAPONTYPE_DILDO2, "Dildo"},                {WEAPONTYPE_VIBE1, "Vibrator"},
    {WEAPONTYPE_VIBE2, "Silver Vibrator"},       {WEAPONTYPE_FLOWERS, "Flowers"},
    {WEAPONTYPE_CANE, "Cane"},                   {WEAPONTYPE_GRENADE, "Grenade"},
    {WEAPONTYPE_TEARGAS, "Tear Gas"},            {WEAPONTYPE_MOLOTOV, "Molotov"},
    {(eWeaponType)22, "Pistol"},                 {(eWeaponType)23, "Silenced Pistol"},
    {(eWeaponType)24, "Desert Eagle"},           {(eWeaponType)25, "Shotgun"},
    {(eWeaponType)26, "Sawn-off Shotgun"},       {(eWeaponType)27, "Combat Shotgun"},
    {(eWeaponType)28, "Micro Uzi"},              {(eWeaponType)29, "MP5"},
    {(eWeaponType)30, "AK-47"},                  {(eWeaponType)31, "M4"},
    {(eWeaponType)32, "Tec-9"},                  {(eWeaponType)33, "Country Rifle"},
    {(eWeaponType)34, "Sniper Rifle"},           {(eWeaponType)35, "Rocket Launcher"},
    {(eWeaponType)36, "Heat-Seeking RPG"},       {(eWeaponType)37, "Flamethrower"},
    {(eWeaponType)38, "Minigun"},                {(eWeaponType)39, "Satchel Charges"},
    {(eWeaponType)41, "Spraycan"},               {(eWeaponType)42, "Fire Extinguisher"},
    {(eWeaponType)43, "Camera"},                 {(eWeaponType)44, "Night Vision"},
    {(eWeaponType)45, "Thermal Goggles"},        {(eWeaponType)46, "Parachute"},
};
const int kCount = sizeof(kWeapons) / sizeof(kWeapons[0]);

void LoadModel(int modelIndex) {
    if (modelIndex <= 0)
        return;
    CStreaming::RequestModel(modelIndex, GAME_REQUIRED);
    CStreaming::LoadAllRequestedModels(false);
}

} // namespace

void GiveWeapon::LoadConfig(const Config& config, const char* section) {
    m_ammo = config.GetInt(section, "Ammo", m_ammo);
    int start = config.GetInt(section, "Weapon", 38); // Minigun
    for (int i = 0; i < kCount; ++i)
        if (kWeapons[i].type == start)
            m_index = i;
}

const char* GiveWeapon::PickerValue() const {
    return kWeapons[m_index].name;
}

void GiveWeapon::PickerStep(int delta) {
    m_index = ((m_index + delta) % kCount + kCount) % kCount;
}

void GiveWeapon::OnActivate(CPlayerPed* player) {
    static char message[64]; // static: CMessages keeps the pointer while it's on screen
    const WeaponEntry& weapon = kWeapons[m_index];

    // weapons with a model need it loaded first (otherwise CJ holds an invisible gun)
    if (CWeaponInfo* info = CWeaponInfo::GetWeaponInfo(weapon.type, 1))
        LoadModel(info->m_nModelId);
    if (weapon.type == (eWeaponType)39) // satchel charges come with the detonator
        if (CWeaponInfo* detonator = CWeaponInfo::GetWeaponInfo((eWeaponType)40, 1))
            LoadModel(detonator->m_nModelId);

    bool melee = weapon.type < WEAPONTYPE_GRENADE || weapon.type >= (eWeaponType)44;
    player->GiveWeapon(weapon.type, melee ? 1 : m_ammo, true);
    player->SetCurrentWeapon(weapon.type);

    std::snprintf(message, sizeof(message), "Weapon: ~y~%s", weapon.name);
    CMessages::AddMessageJumpQ(message, 1500, 0);
}
