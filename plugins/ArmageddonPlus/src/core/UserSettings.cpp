#include "core/UserSettings.h"

#include "core/Config.h"
#include "core/Energy.h"
#include "core/Log.h"
#include "core/PowerManager.h"

#include "plugin.h"

#include <windows.h>

#include <string>

namespace {

std::string Path() {
    return plugin::paths::GetGameDirRelativePathA("ArmageddonPlus.user.ini");
}

void Write(const char* section, const char* key, const char* value) {
    WritePrivateProfileStringA(section, key, value, Path().c_str());
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
    }
    Energy::SetEnabled(user.GetBool("Settings", "Energy", Energy::IsEnabled()));
    powers.SetOverlayEnabled(user.GetBool("Settings", "Overlay", powers.IsOverlayEnabled()));
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

} // namespace UserSettings
