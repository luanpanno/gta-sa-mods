#include "core/Leap.h"

#include "core/FallGuard.h"

#include "plugin.h"
#include "common.h" // GAME_GRAVITY
#include "CPlayerPed.h"
#include "CTimer.h"
#include "CWorld.h"
#include "CWaterLevel.h"

#include <algorithm>

namespace {
const float kSoftLandingSpeed = 0.12f; // m/step (= 6 m/s): touchdown speed, gentle enough to land on his feet
bool g_active = false;
float g_vertical = 0.0f;   // current upward speed of our arc (m/step)
CVector g_horizontal;      // horizontal speed to hold (m/step)
bool g_holdHorizontal = false;
int g_frames = 0;
CVector g_takeOff;         // the game's own speed at take-off, before any power changed it
}

namespace Leap {

void Start(CPlayerPed* player, float verticalSpeed, const CVector& horizontal) {
    if (!g_active || g_frames > 0) // first Start of this jump (a second power may restart it this frame)
        g_takeOff = player->m_vecMoveSpeed;
    g_active = true;
    g_vertical = verticalSpeed;
    g_horizontal = CVector(horizontal.x, horizontal.y, 0.0f);
    g_holdHorizontal = g_horizontal.Magnitude() > 0.0001f;
    g_frames = 0;
    player->m_vecMoveSpeed.z = verticalSpeed;
    if (g_holdHorizontal) {
        player->m_vecMoveSpeed.x = g_horizontal.x;
        player->m_vecMoveSpeed.y = g_horizontal.y;
    }
    FallGuard::Protect(player);
}

void Update(CPlayerPed* player) {
    if (!g_active || !player)
        return;
    ++g_frames;
    bool landed = g_frames > 3 && player->bIsStanding;
    if (landed || player->bInVehicle || !player->bApplyGravity || player->m_fHealth <= 0.0f) {
        g_active = false; // landed, got in a car, started flying or died
        return;
    }

    // rising: follow the clean arc; once at the top, let the game's physics handle the fall
    g_vertical -= GAME_GRAVITY * CTimer::ms_fTimeStep;
    if (g_vertical > 0.0f)
        player->m_vecMoveSpeed.z = g_vertical;

    // falling: soft landing. The game picks the landing animation from the impact speed; a hard
    // impact = falls over + gets up. In the last few meters we cap the fall speed, easing down to
    // ~6 m/s at touchdown, so he lands on his feet.
    if (player->m_vecMoveSpeed.z < 0.0f) {
        const CVector& pos = player->GetPosition(); // ped position = about 1 m above his feet
        bool found = false;
        float ground = CWorld::FindGroundZFor3DCoord(pos.x, pos.y, pos.z, &found, nullptr);
        float water = 0.0f;
        if (CWaterLevel::GetWaterLevelNoWaves(pos.x, pos.y, pos.z, &water) && (!found || water > ground)) {
            ground = water; // landing on water (or on it, with Super Speed's water walk)
            found = true;
        }
        float height = pos.z - 1.0f - ground;
        if (found && height < 6.0f) {
            float allowed = -(kSoftLandingSpeed + std::max(0.0f, height) * 0.05f); // m/step
            if (player->m_vecMoveSpeed.z < allowed)
                player->m_vecMoveSpeed.z = allowed;
        }
    }
    if (g_holdHorizontal) {
        player->m_vecMoveSpeed.x = g_horizontal.x;
        player->m_vecMoveSpeed.y = g_horizontal.y;
    }
}

bool IsActive() {
    return g_active;
}

CVector TakeOffSpeed(CPlayerPed* player) {
    return g_active && g_frames == 0 ? g_takeOff : player->m_vecMoveSpeed;
}

} // namespace Leap
