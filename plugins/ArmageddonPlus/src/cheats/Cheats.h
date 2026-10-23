#pragma once

// Every built-in GTA SA cheat, usable from the menu without typing codes.
//
// How: the game keeps two tables in CCheat:
//   m_aCheatFunctions[id]  code to run (weapon sets, spawning a tank...); null for pure on/off cheats
//   m_aCheatsActive[id]    on/off flag that the rest of the game checks (infinite ammo, pink traffic...)
// Applying a cheat does what typing its code does: run its function if it has one, otherwise flip
// its flag and let CCheat::HandleSpecialCheats fix up mutually exclusive ones.
namespace Cheats {

struct Info {
    int id;            // eCheats
    const char* name;  // readable name
    const char* group; // menu section
    bool oneTime;      // true = happens once (weapons, spawns, weather); false = stays on until toggled off
};

int Count();
const Info& Get(int index);

bool IsActive(int index);   // toggle cheats: currently on
void Apply(int index);      // like typing the code
int ActiveCount();          // toggle cheats currently on

} // namespace Cheats
