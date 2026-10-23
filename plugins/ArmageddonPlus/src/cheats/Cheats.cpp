#include "cheats/Cheats.h"

#include "core/Log.h"

#include "plugin.h"
#include "CCheat.h"

namespace {

const char* kPlayer = "Player";
const char* kWeapons = "Weapons";
const char* kWanted = "Wanted level";
const char* kVehicles = "Spawn vehicles";
const char* kDriving = "Cars & traffic";
const char* kPeople = "People & gangs";
const char* kWorld = "Weather & time";

// order inside each group = order in the menu
const Cheats::Info kCheats[] = {
    // player
    {CHEAT_HEALTH_ARMOR_250K, "Health, armor + $250k", kPlayer, true},
    {CHEAT_INFINITE_HEALTH, "Infinite health", kPlayer, false},
    {CHEAT_INFINITE_OXYGEN, "Infinite oxygen", kPlayer, false},
    {CHEAT_MEGAJUMP, "Mega jump", kPlayer, false},
    {CHEAT_MEGA_PUNCH, "Mega punch", kPlayer, false},
    {CHEAT_NEVER_GET_HUNGRY, "Never get hungry", kPlayer, false},
    {CHEAT_ADRENALINE_MODE, "Adrenaline mode", kPlayer, false},
    {CHEAT_FAT_PLAYER, "Fat CJ", kPlayer, true},
    {CHEAT_MAX_MUSCLE, "Max muscle", kPlayer, true},
    {CHEAT_SKINNY_PLAYER, "Skinny CJ", kPlayer, true},
    {CHEAT_MAX_STAMINA, "Max stamina", kPlayer, true},
    {CHEAT_MAX_RESPECT, "Max respect", kPlayer, true},
    {CHEAT_MAX_SEX_APPEAL, "Max sex appeal", kPlayer, true},
    {CHEAT_MAX_DRIVING_SKILLS, "Max driving skills", kPlayer, true},
    {CHEAT_GET_PARACHUTE, "Get parachute", kPlayer, true},
    {CHEAT_GET_JETPACK, "Get jetpack", kPlayer, true},
    {CHEAT_SUICIDE, "Suicide", kPlayer, true},
    // weapons
    {CHEAT_WEAPON_SET1, "Weapon set 1 (thug)", kWeapons, true},
    {CHEAT_WEAPON_SET2, "Weapon set 2 (professional)", kWeapons, true},
    {CHEAT_WEAPON_SET3, "Weapon set 3 (nutter)", kWeapons, true},
    {CHEAT_INFINITE_AMMO, "Infinite ammo, no reload", kWeapons, false},
    {CHEAT_HITMAN_LEVEL_FOR_ALL_WEAPONS, "Hitman level, all weapons", kWeapons, true},
    {CHEAT_WEAPON_AIMING_WHILE_DRIVING, "Full aiming while driving", kWeapons, false},
    // wanted
    {CHEAT_I_DO_AS_I_PLEASE, "Never wanted", kWanted, false},
    {CHEAT_CLEAR_WANTED_LEVEL, "Clear wanted level", kWanted, true},
    {CHEAT_WANTED_LEVEL_2STARS, "Wanted level +2 stars", kWanted, true},
    {CHEAT_SIX_WANTED_STARS, "Six wanted stars", kWanted, true},
    // spawn vehicles
    {CHEAT_SPAWN_RHINO, "Rhino", kVehicles, true},
    {CHEAT_SPAWN_HUNTER, "Hunter", kVehicles, true},
    {CHEAT_SPAWN_HYDRA, "Hydra", kVehicles, true},
    {CHEAT_SPAWN_VORTEX, "Vortex hovercraft", kVehicles, true},
    {CHEAT_SPAWN_MONSTER, "Monster truck", kVehicles, true},
    {CHEAT_SPAWN_STUNT_PLANE, "Stunt plane", kVehicles, true},
    {CHEAT_SPAWN_QUAD, "Quad", kVehicles, true},
    {CHEAT_SPAWN_TANKER_TRUCK, "Tanker truck", kVehicles, true},
    {CHEAT_SPAWN_DOZER, "Dozer", kVehicles, true},
    {CHEAT_SPAWN_BLOODRINGBANGER, "Bloodring Banger", kVehicles, true},
    {CHEAT_SPAWN_RANCHER, "Rancher", kVehicles, true},
    {CHEAT_SPAWN_HOTRINGA, "Hotring Racer A", kVehicles, true},
    {CHEAT_SPAWN_HOTRINGB, "Hotring Racer B", kVehicles, true},
    {CHEAT_SPAWN_ROMERO, "Romero (hearse)", kVehicles, true},
    {CHEAT_SPAWN_STRETCH, "Stretch limo", kVehicles, true},
    {CHEAT_SPAWN_TRASHMASTER, "Trashmaster", kVehicles, true},
    {CHEAT_SPAWN_CADDY, "Caddy", kVehicles, true},
    // cars & traffic
    {CHEAT_PERFECT_HANDLING, "Perfect handling", kDriving, false},
    {CHEAT_ALL_CARS_HAVE_NITRO, "All cars have nitro", kDriving, false},
    {CHEAT_ALL_TAXIS_NITRO, "Taxis have nitro + hop", kDriving, false},
    {CHEAT_CARS_FLY, "Cars fly", kDriving, false},
    {CHEAT_BOATS_FLY, "Boats fly", kDriving, false},
    {CHEAT_CARS_ON_WATER, "Drive on water", kDriving, false},
    {CHEAT_HUGE_BUNNY_HOP, "Huge bunny hop", kDriving, false},
    {CHEAT_INVISIBLE_CAR, "Invisible cars (wheels only)", kDriving, false},
    {CHEAT_SMASH_N_BOOM, "Smash n' boom (cars explode on touch)", kDriving, false},
    {CHEAT_CARS_FLOAT_AWAY_WHEN_HIT, "Cars float away when hit", kDriving, false},
    {CHEAT_BLOW_UP_ALL_CARS, "Blow up all cars", kDriving, true},
    {CHEAT_BGREEN_LIGHTS_CHEAT, "All green lights", kDriving, false},
    {CHEAT_AGGRESSIVE_DRIVERS, "Aggressive drivers", kDriving, false},
    {CHEAT_PINK_TRAFFIC, "Pink traffic", kDriving, false},
    {CHEAT_BLACK_TRAFFIC, "Black traffic", kDriving, false},
    {CHEAT_CHEAP_TRAFFIC, "Cheap cars", kDriving, false},
    {CHEAT_FAST_TRAFFIC, "Fast cars", kDriving, false},
    {CHEAT_COUNTRY_TRAFFIC, "Country cars", kDriving, false},
    {CHEAT_REDUCED_TRAFFIC, "Reduced traffic", kDriving, false},
    // people & gangs
    {CHEAT_RIOT_MODE, "Riot mode", kPeople, false},
    {CHEAT_PEDS_ATTACK_OTHER_WITH_GOLFCLUB, "Peds fight with golf clubs", kPeople, false},
    {CHEAT_PEDS_ATTACK_YOU_WITH_ROCKETS, "Peds attack you with rockets", kPeople, false},
    {CHEAT_HAVE_ABOUNTY_ON_YOUR_HEAD, "Bounty on your head", kPeople, false},
    {CHEAT_EVERYONE_ARMED, "Everyone is armed", kPeople, false},
    {CHEAT_GANGMEMBERS_EVERYWHERE, "Gang members everywhere", kPeople, false},
    {CHEAT_GANGS_CONTROLS_THE_STREETS, "Gangs control the streets", kPeople, false},
    {CHEAT_WANNA_BE_IN_MY_GANG, "Recruit anyone (9mm)", kPeople, false},
    {CHEAT_NO_ONE_CAN_STOP_US, "Recruit anyone (AK-47)", kPeople, false},
    {CHEAT_ROCKET_MAYHEM, "Recruit anyone (rockets)", kPeople, false},
    {CHEAT_ELVIS_IS_EVERYWHERE, "Elvis is everywhere", kPeople, false},
    {CHEAT_BEACH_PARTY, "Beach party", kPeople, false},
    {CHEAT_NINJA_THEME, "Ninja theme", kPeople, false},
    {CHEAT_FUNHOUSE_THEME, "Funhouse theme", kPeople, false},
    {CHEAT_SLUT_MAGNET, "Hookers follow you", kPeople, false},
    {CHEAT_PROSTITUTES_PAY_YOU, "Hookers pay you", kPeople, false},
    // weather & time
    {CHEAT_SUNNY_WEATHER, "Sunny weather", kWorld, true},
    {CHEAT_VERY_SUNNY_WEATHER, "Very sunny weather", kWorld, true},
    {CHEAT_OVERCAST_WEATHER, "Overcast weather", kWorld, true},
    {CHEAT_RAINY_WEATHER, "Rainy weather", kWorld, true},
    {CHEAT_FOGGY_WEATHER, "Foggy weather", kWorld, true},
    {CHEAT_THUNDER_STORM, "Thunderstorm", kWorld, true},
    {CHEAT_SAND_STORM, "Sandstorm", kWorld, true},
    {CHEAT_ALWAYS_MIDNIGHT, "Always midnight", kWorld, false},
    {CHEAT_STOP_GAME_CLOCK_ORANGE_SKY, "Orange sky, clock stopped", kWorld, false},
    {CHEAT_FASTER_CLOCK, "Faster clock", kWorld, false},
    {CHEAT_FASTER_GAMEPLAY, "Faster gameplay", kWorld, false},
    {CHEAT_SLOWER_GAMEPLAY, "Slower gameplay", kWorld, false},
};
const int kCount = sizeof(kCheats) / sizeof(kCheats[0]);

} // namespace

namespace Cheats {

int Count() {
    return kCount;
}

const Info& Get(int index) {
    return kCheats[index];
}

bool IsActive(int index) {
    return CCheat::m_aCheatsActive[kCheats[index].id];
}

void Apply(int index) {
    int id = kCheats[index].id;
    CCheat::m_bHasPlayerCheated = true; // same as typing a code
    if (CCheat::m_aCheatFunctions[id]) {
        CCheat::m_aCheatFunctions[id]();  // e.g. WeaponCheat1, TankCheat, MayhemCheat (toggles its own flag)
    } else {
        CCheat::m_aCheatsActive[id] = !CCheat::m_aCheatsActive[id];
        CCheat::HandleSpecialCheats(id); // turns off conflicting cheats (pink vs black traffic...)
    }
    Log::Info("Cheat: %s (%s)", kCheats[index].name,
              kCheats[index].oneTime ? "used" : (CCheat::m_aCheatsActive[id] ? "on" : "off"));
}

int ActiveCount() {
    int count = 0;
    for (int i = 0; i < kCount; ++i)
        if (!kCheats[i].oneTime && IsActive(i))
            ++count;
    return count;
}

} // namespace Cheats
