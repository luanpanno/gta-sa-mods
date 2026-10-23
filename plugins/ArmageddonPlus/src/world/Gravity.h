#pragma once

#include "core/Power.h"

// Phase 5.1: world gravity. Changes gravity for EVERYTHING: CJ, people, cars, objects, explosions'
// debris. Levels: Moon (17%), Mars (38%), Zero-G (almost none), Heavy (250%).
//
// How: the game reads one global constant for gravity, GAME_GRAVITY (0.008 m/step², at 0x863984),
// in CPhysical::ApplyGravity. We overwrite it every frame and put the original back when it's off.
class Gravity : public Power {
public:
    static constexpr int kLevels = 4;

    const char* Name() const override { return "Gravity"; }
    const char* Id() const override { return "Gravity"; }
    const char* Description() const override { return "Gravity for everything: people, cars, objects"; }
    const char* Category() const override { return "World"; }
    int MaxLevel() const override { return kLevels; }
    const char* LevelName(int level) const override;

    void LoadConfig(const Config& config, const char* section) override;
    void OnActivate(CPlayerPed* player) override;
    void Update(CPlayerPed* player, float dt) override;
    void OnDeactivate(CPlayerPed* player) override;

private:
    float m_scale[kLevels] = {0.17f, 0.38f, 0.02f, 2.5f};
    float m_original = 0.008f;
};
