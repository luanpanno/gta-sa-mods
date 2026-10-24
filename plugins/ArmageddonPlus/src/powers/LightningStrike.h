#pragma once

#include "core/Power.h"
#include "CVector.h"

#include <vector>

// Phase 3.9: Lightning Strike. Turn it on in the menu, then point the camera (the crosshair in the
// middle of the screen) and click LMB: a lightning bolt hits that spot and blows it up.
// 3 levels = blast size (grenade / rocket / car bomb + fire).
//
// How: a ray from the camera finds the spot; CExplosion::AddExplosion does the damage.
// The bolt is drawn by us: a jagged line of points from the sky down to the spot, projected to the
// screen (CSprite::CalcScreenCoors) and drawn as thin quads, plus a white screen flash.
class LightningStrike : public Power {
public:
    static constexpr int kLevels = 3;
    static constexpr int kBoltPoints = 14;

    const char* Name() const override { return "Lightning Strike"; }
    const char* Id() const override { return "LightningStrike"; }
    const char* Description() const override { return "Point the center crosshair and click LMB to call down lightning"; }
    int MaxLevel() const override { return kLevels; }
    const char* ExclusiveGroup() const override { return "mouse"; }

    void LoadConfig(const Config& config, const char* section) override;
    void OnActivate(CPlayerPed* player) override;
    void Update(CPlayerPed* player, float dt) override;
    void OnDeactivate(CPlayerPed* player) override;
    void Draw() override;

private:
    struct Bolt {
        CVector points[kBoltPoints]; // sky -> ground
    };

    void Strike(CPlayerPed* player);
    void AddBolt(const CVector& target);
    void DrawBolt(const Bolt& bolt, float age);
    void DrawCrosshair();

    // per level (index 0 = level 1)
    int m_boltCount[kLevels] = {2, 5, 10};               // bolts per strike
    float m_spread[kLevels] = {6.0f, 14.0f, 24.0f};      // m: how far extra bolts land from the target
    float m_shockRadius[kLevels] = {22.0f, 35.0f, 55.0f};
    float m_shockSpeed[kLevels] = {40.0f, 65.0f, 100.0f}; // m/s at the center

    float m_range = 300.0f;     // m
    float m_cooldown = 0.8f;    // real seconds between strikes
    char m_hadFireDisabled = 0;
    bool m_wasExplosionProof = false;

    unsigned long long m_lastStrikeMs = 0;  // real time (unaffected by Bullet Time)
    std::vector<Bolt> m_bolts;              // bolts of the last strike, drawn for a moment
};
