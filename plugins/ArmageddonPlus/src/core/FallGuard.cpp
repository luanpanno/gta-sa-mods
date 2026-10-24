#include "core/FallGuard.h"

#include "plugin.h"
#include "CPlayerPed.h"

namespace {
bool g_protecting = false;
bool g_wasCollisionProof = false; // CJ's state before the first Protect()
float g_airTime = 0.0f;           // frames since Protect(), so the take-off frame doesn't count as landing
}

namespace FallGuard {

void Protect(CPlayerPed* player) {
    if (!g_protecting) {
        g_wasCollisionProof = player->bCollisionProof;
        g_protecting = true;
    }
    g_airTime = 0.0f;
    player->bCollisionProof = true;
}

void Update(CPlayerPed* player) {
    if (!g_protecting || !player)
        return;
    g_airTime += 1.0f;
    player->bCollisionProof = true; // the game may reset it mid-air
    bool landed = g_airTime > 3.0f && (player->bIsStanding || player->bTouchingWater);
    if (landed || player->bInVehicle || player->m_fHealth <= 0.0f) {
        player->bCollisionProof = g_wasCollisionProof;
        g_protecting = false;
    }
}

bool IsProtecting() {
    return g_protecting;
}

} // namespace FallGuard
