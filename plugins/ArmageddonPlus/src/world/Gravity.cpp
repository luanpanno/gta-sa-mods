#include "world/Gravity.h"

#include "core/Config.h"

#include "plugin.h"
#include "common.h" // GAME_GRAVITY

#include <cstdio>

const char* Gravity::LevelName(int level) const {
    static const char* kNames[kLevels] = {"Moon", "Mars", "Zero-G", "Heavy"};
    return level >= 1 && level <= kLevels ? kNames[level - 1] : nullptr;
}

void Gravity::LoadConfig(const Config& config, const char* section) {
    static const char* kKeys[kLevels] = {"Moon", "Mars", "ZeroG", "Heavy"};
    for (int i = 0; i < kLevels; ++i)
        m_scale[i] = config.GetFloat(section, kKeys[i], m_scale[i]);
}

void Gravity::OnActivate(CPlayerPed*) {
    m_original = GAME_GRAVITY; // normally 0.008
}

void Gravity::Update(CPlayerPed*, float) {
    GAME_GRAVITY = m_original * m_scale[Level() - 1]; // re-applied every frame (cheap, and survives resets)
}

void Gravity::OnDeactivate(CPlayerPed*) {
    GAME_GRAVITY = m_original;
}
