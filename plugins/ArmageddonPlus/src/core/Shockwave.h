#pragma once

#include "CVector.h"

class CPed;

// A physical shockwave: every ped, vehicle and object within `radius` of `center` gets a velocity
// away from it (or toward it, for a pull), strongest in the middle, plus some lift so things fly.
// Used by Force push/pull, Explosion Punch and Lightning Strike.
//
// How: set CPhysical::m_vecMoveSpeed directly; parked cars / placed objects are woken first
// (SetIsStatic(false)); peds are lifted a bit so physics, not their walk animation, moves them.
namespace Shockwave {

struct Params {
    float radius = 10.0f;     // m
    float speed = 20.0f;      // m/s at the center
    float edgeFactor = 0.4f;  // speed at the edge, as a fraction of the center speed
    float lift = 0.35f;       // upward speed, as a fraction of the outward speed
    bool pull = false;        // toward the center instead of away
    const CVector* cone = nullptr; // if set: only things in this direction (unit vector, horizontal)
    float coneDot = 0.3f;     // ~140 degree cone
};

// Returns how many things were hit. `ignore` (usually CJ) and his vehicle are never affected.
int Blast(const CVector& center, const Params& params, CPed* ignore);

} // namespace Shockwave
