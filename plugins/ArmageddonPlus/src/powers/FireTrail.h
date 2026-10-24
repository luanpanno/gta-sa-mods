#pragma once

#include "core/Power.h"
#include "CVector.h"

class CVehicle;

// Phase 3.8: Fire Trail. Everywhere CJ runs (or drives) catches fire behind him; he's fireproof.
// 3 levels = how long the flames last (2 s / 4 s / 8 s); level 3 flames can also spread.
// Pairs well with Super Speed.
//
// How: every couple of meters CJ moves, gFireManager.StartFire() lights a fire at his feet
// (or behind his car), with CJ as the "creator" so the game credits him for what burns.
class FireTrail : public Power {
public:
    static constexpr int kLevels = 3;

    const char* Name() const override { return "Fire Trail"; }
    const char* Id() const override { return "FireTrail"; }
    const char* Description() const override { return "Leave a trail of fire on foot or behind your car; you are fireproof"; }
    int MaxLevel() const override { return kLevels; }

    void LoadConfig(const Config& config, const char* section) override;
    void OnActivate(CPlayerPed* player) override;
    void Update(CPlayerPed* player, float dt) override;
    void OnDeactivate(CPlayerPed* player) override;
    bool IsDraining() const override { return m_moving; } // standing still costs nothing

private:
    void SetVehicleFireproof(CVehicle* vehicle);

    float m_duration[kLevels] = {2.0f, 4.0f, 8.0f}; // seconds a flame burns
    int m_spread[kLevels] = {0, 0, 1};              // "generations": how many times a flame may spread
    float m_spacing = 1.5f;                         // meters between flames

    CVector m_lastFire;
    bool m_moving = false; // moving this frame (drives energy drain)
    bool m_hasLastFire = false;
    bool m_wasFireProof = false;
    CVehicle* m_vehicle = nullptr; // vehicle we made fireproof
    bool m_vehicleWasFireProof = false;
};
