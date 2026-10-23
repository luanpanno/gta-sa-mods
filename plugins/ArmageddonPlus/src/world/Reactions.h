#pragma once

#include "core/Power.h"

#include <unordered_map>

class CPed;

// Phase 5.4: the world reacts to CJ's powers (only while at least one superpower is on).
//
//   1 Fear            nearby pedestrians run away from CJ
//   2 Fear + Police   ...and the police come: wanted level at least 2 stars
//
// How: script commands, the same ones missions use. TASK_SMART_FLEE_CHAR gives a ped the
// "flee from this character" task (they also scream / panic on their own), and
// ALTER_WANTED_LEVEL_NO_DROP raises the wanted level without ever lowering it.
// Mission characters are left alone.
class Reactions : public Power {
public:
    static constexpr int kLevels = 2;

    const char* Name() const override { return "NPC Reactions"; }
    const char* Id() const override { return "Reactions"; }
    const char* Category() const override { return "World"; }
    int MaxLevel() const override { return kLevels; }
    const char* LevelName(int level) const override;

    void LoadConfig(const Config& config, const char* section) override;
    void Update(CPlayerPed* player, float dt) override;
    void OnDeactivate(CPlayerPed* player) override;

private:
    float m_radius = 35.0f;      // m: who notices
    int m_policeLevel = 2;       // stars at level 2
    float m_timer = 0.0f;
    float m_clock = 0.0f;        // seconds since on, to remember when each ped was told to flee
    std::unordered_map<CPed*, float> m_fledAt; // don't re-task the same ped every second
};
