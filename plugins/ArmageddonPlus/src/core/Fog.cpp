#include "core/Fog.h"

#include "plugin.h"
#include "CTimeCycle.h"

namespace {
bool g_enabled = true; // the game's normal fog
}

namespace Fog {

void SetEnabled(bool enabled) {
    g_enabled = enabled;
}

bool IsEnabled() {
    return g_enabled;
}

void Update() {
    if (g_enabled)
        return;
    CColourSet& colours = CTimeCycle::m_CurrentColours;
    // just short of the draw distance: the far edge still fades out instead of cutting off hard
    float start = colours.m_fFarClip * 0.95f;
    if (colours.m_fFogStart < start)
        colours.m_fFogStart = start;
}

} // namespace Fog
