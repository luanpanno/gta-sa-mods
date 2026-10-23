#include "core/Power.h"

#include "core/Energy.h"

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
