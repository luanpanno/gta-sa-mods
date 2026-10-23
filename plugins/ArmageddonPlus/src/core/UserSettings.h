#pragma once

class PowerManager;

// Your personal settings, changed from inside the game and kept between sessions:
// power shortcuts (bound in the menu), energy on/off, power list on/off.
//
// Stored in C:\Games\GTA-SA-Modding\ArmageddonPlus.user.ini, NOT next to the .asi: deploy.ps1 overwrites
// the plugin folder's ArmageddonPlus.ini, but never touches this file. Values here win over the main ini.
namespace UserSettings {

void Apply(PowerManager& powers); // call after loading the main ini
void SaveKey(const char* powerId, int vk); // vk 0 = no shortcut
void SaveEnergyEnabled(bool enabled);
void SaveOverlayEnabled(bool enabled);

} // namespace UserSettings
