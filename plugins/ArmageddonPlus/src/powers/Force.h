#pragma once

#include "core/Power.h"

// Phase 3.6: Force Push & Pull. 3 levels = radius / strength. Turn it on in the menu, then (on foot):
//
//   LMB   push: shockwave that knocks everything in front of CJ away
//   RMB   pull: yanks everything in front of CJ toward him
//
// Only one "mouse" power (this or Telekinesis) can be on at a time. CJ can't shoot while it's on.
//
// How: every ped / vehicle / object within the radius and inside a ~140 degree cone in front of the
// camera gets a velocity (m_vecMoveSpeed) away from / toward CJ, plus some lift so things fly.
class Force : public Power {
public:
    static constexpr int kLevels = 3;

    const char* Name() const override { return "Force Push / Pull"; }
    const char* Id() const override { return "Force"; }
    const char* Description() const override { return "LMB pushes everything in front away, RMB pulls it toward you"; }
    int MaxLevel() const override { return kLevels; }
    const char* ExclusiveGroup() const override { return "mouse"; }

    void LoadConfig(const Config& config, const char* section) override;
    void OnActivate(CPlayerPed* player) override;
    void Update(CPlayerPed* player, float dt) override;
    void OnDeactivate(CPlayerPed* player) override;

private:
    void Blast(CPlayerPed* player, bool pull);

    float m_radius[kLevels] = {14.0f, 24.0f, 40.0f};   // m
    float m_pushSpeed[kLevels] = {25.0f, 45.0f, 80.0f}; // m/s at the center
    float m_pullSpeed[kLevels] = {16.0f, 26.0f, 40.0f}; // m/s
    float m_cooldown = 0.6f; // seconds between blasts
    float m_cooldownLeft = 0.0f;
    char m_hadFireDisabled = 0;
};
