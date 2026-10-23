#include "core/Fx.h"

#include "core/Config.h"

#include "plugin.h"
#include "CWorld.h"
#include "FxManager_c.h"
#include "FxSystem_c.h"

namespace {
bool g_enabled = true;
}

namespace Fx {

void LoadConfig(const Config& config) {
    g_enabled = config.GetBool("Effects", "Particles", true);
}

bool IsEnabled() {
    return g_enabled;
}

void Spawn(const char* name, const CVector& position) {
    if (!g_enabled)
        return;
    RwV3d pos{position.x, position.y, position.z};
    // ignoreBoundingChecks = true: spawn it even if the camera isn't looking there yet
    FxSystem_c* fx = g_fxMan.CreateFxSystem(const_cast<char*>(name), &pos, nullptr, true);
    if (fx)
        fx->PlayAndKill(); // play once, then the game deletes it
}

bool GroundBelow(const CVector& position, float maxDrop, CVector& ground) {
    bool found = false;
    float z = CWorld::FindGroundZFor3DCoord(position.x, position.y, position.z, &found, nullptr);
    if (!found || position.z - z > maxDrop)
        return false;
    ground = CVector(position.x, position.y, z);
    return true;
}

} // namespace Fx
