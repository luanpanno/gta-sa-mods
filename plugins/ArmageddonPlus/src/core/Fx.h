#pragma once

#include "CVector.h"

class Config;

// Phase 4.3: one-off particle effects, using the game's own effect systems (models\effects.fxp):
// "heli_dust", "wallbust", "camflash", "explosion_tiny", "spraycan"... Same thing the game uses for
// helicopter downdraft, bullet dust, camera flashes.
namespace Fx {

void LoadConfig(const Config& config);
bool IsEnabled();

// Spawn an effect at a world position; it plays once and cleans itself up.
void Spawn(const char* name, const CVector& position);

// Ground height below a point (within maxDrop meters). false if there's no ground that close.
bool GroundBelow(const CVector& position, float maxDrop, CVector& ground);

} // namespace Fx
