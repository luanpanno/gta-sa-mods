#pragma once

#include "core/Power.h"
#include "CVector.h"

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
    int MaxLevel() const override { return kLevels; }
    const char* ExclusiveGroup() const override { return "mouse"; }

    void LoadConfig(const Config& config, const char* section) override;
    void OnActivate(CPlayerPed* player) override;
    void Update(CPlayerPed* player, float dt) override;
    void OnDeactivate(CPlayerPed* player) override;
    void Draw() override;

private:
    void Strike(CPlayerPed* player);
    void DrawBolt(float age);
    void DrawCrosshair();

    float m_range = 300.0f;     // m
    float m_cooldown = 0.8f;    // real seconds between strikes
    char m_hadFireDisabled = 0;

    unsigned long long m_lastStrikeMs = 0;  // real time (unaffected by Bullet Time)
    CVector m_bolt[kBoltPoints];            // sky -> ground
    bool m_hasBolt = false;
};
