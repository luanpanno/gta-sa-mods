#pragma once

class PowerManager;

// The always-on HUD:
//
//   top-right (hidden while the menu is open, the menu shows everything):
//     ARMAGEDDON+                    title
//     [X] Flight          ■■□        powers that are ON or have a shortcut, with key badge + level
//     [Z] Super Speed     ■□□        (shortcut-only powers are dimmed)
//
//   bottom center (always, when the energy system is on):
//     ENERGY [■■■■■■■■□□□] 87
namespace Hud {

void Draw(PowerManager& powers); // top-right block
void DrawEnergy();               // bottom-center energy bar

} // namespace Hud
