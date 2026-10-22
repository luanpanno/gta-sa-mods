#pragma once

#include <string>

// Reads ArmageddonPlus.ini (deployed next to the .asi, from config/ArmageddonPlus.ini in the repo).
// Missing keys fall back to the default passed in, so the plugin works without the file too.
class Config {
public:
    void Load(const std::string& path);

    int GetInt(const char* section, const char* key, int defaultValue) const;
    float GetFloat(const char* section, const char* key, float defaultValue) const;
    bool GetBool(const char* section, const char* key, bool defaultValue) const;
    // Key names like "F2", "X", "NUMPAD5", "SHIFT" or "0x70" -> Windows virtual-key code. 0 = unbound.
    int GetKey(const char* section, const char* key, int defaultVk) const;

    static const char* KeyName(int vk); // for on-screen hints, e.g. 0x71 -> "F2"

private:
    std::string GetString(const char* section, const char* key) const;
    std::string m_path;
};
