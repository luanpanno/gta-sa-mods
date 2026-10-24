#pragma once

class CPlayerPed;

// Fall-damage protection for big jumps, shared by every power that launches CJ (Super Speed, Super Jump).
//
// Protect() makes CJ collision-proof until he next touches the ground; Update() (every frame, from
// main) restores his ORIGINAL state when he lands. Shared so two powers jumping at once can't save
// each other's "proof" state and leave him invulnerable for good.
namespace FallGuard {

void Protect(CPlayerPed* player);
void Update(CPlayerPed* player);
bool IsProtecting();

} // namespace FallGuard
