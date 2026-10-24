#pragma once

#include "CVector.h"

class CPlayerPed;

// Big jumps that actually go high: a ballistic arc we drive ourselves.
//
// Why: setting CJ's upward speed once at take-off isn't enough. Airborne peds get damped (air
// resistance, the in-air task) so most of a big kick is lost within a few frames. Instead, from
// take-off until the top of the arc we SET his vertical speed every frame to what a clean throw
// would have (v0 - gravity * time), and keep his horizontal speed. After the peak, normal physics
// takes over for the fall, except the last few meters, where the fall speed is capped (soft landing:
// a hard impact makes the game play the fall-over + get-up animation). FallGuard protects him until he
// lands, and cancels any get-up animation that still starts.
namespace Leap {

// verticalSpeed in m/step (height ~ v^2 / (2 * 0.008) m at normal gravity);
// horizontal = m/step to keep during the whole jump (zero vector = leave horizontal alone)
void Start(CPlayerPed* player, float verticalSpeed, const CVector& horizontal);
void Update(CPlayerPed* player); // every frame, from main
bool IsActive();
// CJ's speed as the game gave it at take-off. Several powers can react to the same jump (Super Speed,
// then Super Jump); this lets the later one scale the original speed, not the earlier one's boost.
CVector TakeOffSpeed(CPlayerPed* player);

} // namespace Leap
