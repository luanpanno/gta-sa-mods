#pragma once

#include "core/Power.h"

class CVehicle;

// Phase 5.3: chaos modes (one at a time; the level picks the mode):
//
//   1 Meteors            explosions rain down at random spots around CJ
//   2 Exploding Traffic  a random nearby car blows up every few seconds
//   3 Flying Cars        random nearby cars get launched into the air, spinning
//   4 Riot               everyone fights everyone (the game's own riot cheat)
//
// Mission vehicles are never touched, so missions don't break.
class Chaos : public Power {
public:
    static constexpr int kLevels = 4;

    const char* Name() const override { return "Chaos"; }
    const char* Id() const override { return "Chaos"; }
    const char* Description() const override { return "Random mayhem around CJ; mission vehicles are spared"; }
    const char* Category() const override { return "World"; }
    int MaxLevel() const override { return kLevels; }
    const char* LevelName(int level) const override;

    void LoadConfig(const Config& config, const char* section) override;
    void OnLevelChanged(CPlayerPed* player, int level) override;
    void Update(CPlayerPed* player, float dt) override;
    void OnDeactivate(CPlayerPed* player) override;

private:
    CVehicle* RandomVehicle(CPlayerPed* player, float radius); // nearby, not CJ's, not a mission car
    void Meteor(CPlayerPed* player);
    void SetRiot(bool on);

    float m_interval[kLevels] = {0.7f, 2.5f, 1.2f, 0.0f}; // seconds between events
    float m_radius = 60.0f;
    float m_timer = 0.0f;
    bool m_riotOn = false;
    bool m_riotWasOn = false;
};
