#pragma once

// "Fog" switch in the menu. Off = no distance fog: the scene stays clear up to the draw distance.
//
// How: every frame the game blends the time cycle (time of day + weather) into
// CTimeCycle::m_CurrentColours, including where fog starts (m_fFogStart) and the draw distance
// (m_fFarClip). With fog off, we push the fog start out to the draw distance after that blend and
// before the frame is rendered. Turning it back on just stops overriding: the next blend restores it.
namespace Fog {

void SetEnabled(bool enabled);
bool IsEnabled();
void Update(); // every frame, from gameProcessEvent

} // namespace Fog
