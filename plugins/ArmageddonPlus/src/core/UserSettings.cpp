#include "core/UserSettings.h"

#include "core/Config.h"
#include "core/Energy.h"
#include "core/Fog.h"
#include "core/Log.h"
#include "core/PowerManager.h"

#include "plugin.h"

#include <windows.h>

#include <cstdio>
#include <string>

namespace {

std::string Path() {
    return plugin::paths::GetGameDirRelativePathA("ArmageddonPlus.user.ini");
}

void Write(const char* section, const char* key, const char* value) {
    WritePrivateProfileStringA(section, key, value, Path().c_str());
}

std::string Section(const Power& power) {
    return std::string("Power.") + power.Id();
}

} // namespace

namespace UserSettings {

void Apply(PowerManager& powers) {
    Config user;
    user.Load(Path());

    for (size_t i = 0; i < powers.Count(); ++i) {
        Power& power = powers.At(i);
        int vk = user.GetKey("Keys", power.Id(), -1); // -1 = not set here: keep the main ini's
        if (vk >= 0)
            powers.SetKey(power, vk);

        std::string section = Section(power);
        char key[48];
        for (int s = 0; s < power.SettingCount(); ++s) {
            int levels = power.GetSetting(s).perLevel ? Power::kMaxLevels : 1;
            for (int level = 1; level <= levels; ++level) {
                power.SettingKey(s, level, key, sizeof(key));
                power.SetSettingStored(s, level, user.GetFloat(section.c_str(), key, power.SettingStored(s, level)));
            }
        }
    }
    Energy::SetEnabled(user.GetBool("Settings", "Energy", Energy::IsEnabled()));
    powers.SetOverlayEnabled(user.GetBool("Settings", "Overlay", powers.IsOverlayEnabled()));
    Fog::SetEnabled(user.GetBool("Settings", "Fog", Fog::IsEnabled()));
    Log::Info("User settings: %s", Path().c_str());
}

void SaveKey(const char* powerId, int vk) {
    Write("Keys", powerId, vk ? Config::KeyName(vk) : "NONE");
}

void SaveEnergyEnabled(bool enabled) {
    Write("Settings", "Energy", enabled ? "1" : "0");
}

void SaveOverlayEnabled(bool enabled) {
    Write("Settings", "Overlay", enabled ? "1" : "0");
}

void SaveFogEnabled(bool enabled) {
    Write("Settings", "Fog", enabled ? "1" : "0");
}

void SaveSetting(const Power& power, int index, int level) {
    char key[48], value[32];
    power.SettingKey(index, level, key, sizeof(key));
    std::snprintf(value, sizeof(value), "%g", power.SettingStored(index, level));
    Write(Section(power).c_str(), key, value);
}

void ClearSettings(const Power& power) {
    char key[48];
    for (int s = 0; s < power.SettingCount(); ++s) {
        int levels = power.GetSetting(s).perLevel ? Power::kMaxLevels : 1;
        for (int level = 1; level <= levels; ++level) {
            power.SettingKey(s, level, key, sizeof(key));
            Write(Section(power).c_str(), key, nullptr); // nullptr = delete the line
        }
    }
}

} // namespace UserSettings
