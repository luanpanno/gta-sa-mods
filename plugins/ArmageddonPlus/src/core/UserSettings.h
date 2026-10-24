#pragma once

class PowerManager;
class Power;

// Your personal settings, changed from inside the game and kept between sessions:
// power shortcuts (bound in the menu), energy on/off, power list on/off, Customize page values.
//
// Stored in C:\Games\GTA-SA-Modding\ArmageddonPlus.user.ini, NOT next to the .asi: deploy.ps1 overwrites
// the plugin folder's ArmageddonPlus.ini, but never touches this file. Values here win over the main ini.
namespace UserSettings {

void Apply(PowerManager& powers); // call after loading the main ini
void SaveKey(const char* powerId, int vk); // vk 0 = no shortcut
void SaveEnergyEnabled(bool enabled);
void SaveOverlayEnabled(bool enabled);
void SaveFogEnabled(bool enabled);
// Customize page values, in [Power.<Id>] with the main ini's key names (so lines can be copied over)
void SaveSetting(const Power& power, int index, int level);
void ClearSettings(const Power& power); // back to the main ini's values

} // namespace UserSettings
