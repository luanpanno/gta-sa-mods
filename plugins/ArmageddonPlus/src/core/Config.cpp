#include "core/Config.h"

#include <windows.h>

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>

namespace {

struct NamedKey {
    const char* name;
    int vk;
};

const NamedKey kNamedKeys[] = {
    {"SHIFT", VK_SHIFT},   {"CTRL", VK_CONTROL},  {"ALT", VK_MENU},       {"SPACE", VK_SPACE},
    {"TAB", VK_TAB},       {"CAPSLOCK", VK_CAPITAL}, {"INSERT", VK_INSERT}, {"DELETE", VK_DELETE},
    {"HOME", VK_HOME},     {"END", VK_END},       {"PAGEUP", VK_PRIOR},   {"PAGEDOWN", VK_NEXT},
    {"UP", VK_UP},         {"DOWN", VK_DOWN},     {"LEFT", VK_LEFT},      {"RIGHT", VK_RIGHT},
    {"LMB", VK_LBUTTON},   {"RMB", VK_RBUTTON},   {"MMB", VK_MBUTTON},
};

std::string Upper(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return (char)std::toupper(c); });
    return s;
}

int ParseKey(const std::string& raw) {
    std::string name = Upper(raw);
    if (name.empty() || name == "NONE")
        return 0;
    if (name.size() == 1 && (std::isalnum((unsigned char)name[0])))
        return name[0]; // 'A'-'Z' and '0'-'9' are their own VK codes
    if (name[0] == 'F' && name.size() <= 3) {
        int n = std::atoi(name.c_str() + 1);
        if (n >= 1 && n <= 24)
            return VK_F1 + n - 1;
    }
    if (name.rfind("NUMPAD", 0) == 0 && name.size() == 7 && std::isdigit((unsigned char)name[6]))
        return VK_NUMPAD0 + (name[6] - '0');
    if (name.rfind("0X", 0) == 0)
        return (int)std::strtol(name.c_str(), nullptr, 16);
    for (const auto& k : kNamedKeys)
        if (name == k.name)
            return k.vk;
    return -1; // unknown
}

} // namespace

void Config::Load(const std::string& path) {
    m_path = path;
}

std::string Config::GetString(const char* section, const char* key) const {
    char buffer[256] = {};
    GetPrivateProfileStringA(section, key, "", buffer, sizeof(buffer), m_path.c_str());
    std::string value = buffer;
    // strip inline comments ("F2 ; super speed") and surrounding spaces
    auto comment = value.find_first_of(";#");
    if (comment != std::string::npos)
        value.erase(comment);
    while (!value.empty() && std::isspace((unsigned char)value.back()))
        value.pop_back();
    return value;
}

std::string Config::GetText(const char* section, const char* key, const char* defaultValue) const {
    std::string value = GetString(section, key);
    return value.empty() ? defaultValue : value;
}

int Config::GetInt(const char* section, const char* key, int defaultValue) const {
    std::string value = GetString(section, key);
    return value.empty() ? defaultValue : std::atoi(value.c_str());
}

float Config::GetFloat(const char* section, const char* key, float defaultValue) const {
    std::string value = GetString(section, key);
    return value.empty() ? defaultValue : (float)std::atof(value.c_str());
}

bool Config::GetBool(const char* section, const char* key, bool defaultValue) const {
    std::string value = Upper(GetString(section, key));
    if (value.empty())
        return defaultValue;
    return value == "1" || value == "TRUE" || value == "ON" || value == "YES";
}

int Config::GetKey(const char* section, const char* key, int defaultVk) const {
    std::string value = GetString(section, key);
    if (value.empty())
        return defaultVk;
    int vk = ParseKey(value);
    return vk < 0 ? defaultVk : vk;
}

const char* Config::KeyName(int vk) {
    static char buffer[16];
    if (vk == 0)
        return "-";
    if ((vk >= 'A' && vk <= 'Z') || (vk >= '0' && vk <= '9')) {
        std::snprintf(buffer, sizeof(buffer), "%c", vk);
        return buffer;
    }
    if (vk >= VK_F1 && vk <= VK_F24) {
        std::snprintf(buffer, sizeof(buffer), "F%d", vk - VK_F1 + 1);
        return buffer;
    }
    if (vk >= VK_NUMPAD0 && vk <= VK_NUMPAD9) {
        std::snprintf(buffer, sizeof(buffer), "NUMPAD%d", vk - VK_NUMPAD0);
        return buffer;
    }
    for (const auto& k : kNamedKeys)
        if (vk == k.vk)
            return k.name;
    std::snprintf(buffer, sizeof(buffer), "0x%02X", vk);
    return buffer;
}
