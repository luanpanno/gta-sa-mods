#include "core/Power.h"

#include "core/Energy.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

int Power::LevelIndex() const {
    int index = (m_level > 0 ? m_level : 1) - 1;
    return index < kMaxLevels ? index : kMaxLevels - 1;
}

float Power::DrainPerSecond() const {
    return m_drain[LevelIndex()];
}

float Power::CostPerUse() const {
    return m_cost[LevelIndex()];
}

bool Power::PayForUse() const {
    return Energy::TrySpend(CostPerUse());
}

// --- Customize settings ------------------------------------------------------------------------

namespace {

const float kGravity = 0.008f; // GAME_GRAVITY, m/step^2

float ToShown(Power::Display display, float stored) {
    switch (display) {
    case Power::Display::Percent:
        return stored * 100.0f;
    case Power::Display::JumpHeight:
        return stored * stored / (2.0f * kGravity);
    default:
        return stored;
    }
}

float ToStored(Power::Display display, float shown) {
    switch (display) {
    case Power::Display::Percent:
        return shown / 100.0f;
    case Power::Display::JumpHeight:
        return std::sqrt(2.0f * kGravity * (shown > 0.0f ? shown : 0.0f));
    default:
        return shown;
    }
}

} // namespace

void Power::AddSetting(const char* label, const char* key, float* values, bool perLevel, float min, float max,
                       float step, const char* unit, Display display) {
    m_settings.push_back({label, key, values, nullptr, perLevel, min, max, step, unit, display, {}});
}

void Power::AddSetting(const char* label, const char* key, int* values, bool perLevel, int min, int max,
                       const char* unit) {
    m_settings.push_back({label, key, nullptr, values, perLevel, (float)min, (float)max, 1.0f, unit, Display::Plain, {}});
}

bool Power::HasPerLevelSettings() const {
    for (const Setting& setting : m_settings)
        if (setting.perLevel)
            return true;
    return false;
}

float* Power::StoragePtr(int index, int level, int** intOut) const {
    const Setting& setting = m_settings[index];
    int slot = setting.perLevel ? std::clamp(level, 1, kMaxLevels) - 1 : 0;
    *intOut = setting.ints ? setting.ints + slot : nullptr;
    return setting.floats ? setting.floats + slot : nullptr;
}

float Power::SettingStored(int index, int level) const {
    int* i = nullptr;
    float* f = StoragePtr(index, level, &i);
    return f ? *f : (float)*i;
}

void Power::SetSettingStored(int index, int level, float stored) {
    int* i = nullptr;
    float* f = StoragePtr(index, level, &i);
    if (f)
        *f = stored;
    else
        *i = (int)std::lround(stored);
}

float Power::SettingShown(int index, int level) const {
    return ToShown(m_settings[index].display, SettingStored(index, level));
}

void Power::SetSettingShown(int index, int level, float shown) {
    const Setting& setting = m_settings[index];
    shown = std::clamp(shown, setting.min, setting.max);
    SetSettingStored(index, level, ToStored(setting.display, shown));
}

void Power::SettingKey(int index, int level, char* out, int size) const {
    const Setting& setting = m_settings[index];
    if (setting.perLevel)
        std::snprintf(out, size, "%s%d", setting.key, std::clamp(level, 1, kMaxLevels));
    else
        std::snprintf(out, size, "%s", setting.key);
}

void Power::SnapshotSettingDefaults() {
    for (int s = 0; s < SettingCount(); ++s)
        for (int level = 1; level <= kMaxLevels; ++level)
            m_settings[s].defaults[level - 1] = SettingStored(s, level);
}

void Power::ResetSettings() {
    for (int s = 0; s < SettingCount(); ++s)
        for (int level = 1; level <= kMaxLevels; ++level)
            SetSettingStored(s, level, m_settings[s].defaults[level - 1]);
}
