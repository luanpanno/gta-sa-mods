#pragma once

#include "core/Power.h"

#include <unordered_map>

class CEntity;

// Phase 3.3: CJ's melee hits (fists, bat, katana...) make the victim explode. Each punch:
// launches the victim away, a big blast on them + a ring of blasts around, and a shockwave that
// throws everything nearby. Levels scale it all (level 3 = aircraft-size blast + 7-blast ring).
//
// How a hit is detected: the game records on every ped/vehicle who damaged it last
// (m_pLastEntityDamage / m_pLastDamageEntity) and with which weapon. Each frame we look at
// everything near CJ: if its health dropped, CJ was the last attacker and the weapon was melee,
// that was a punch -> explosion at the victim. CJ is explosion-proof while the power is on.
class ExplosionPunch : public Power {
public:
    static constexpr int kLevels = 3;

    const char* Name() const override { return "Explosion Punch"; }
    const char* Id() const override { return "ExplosionPunch"; }
    const char* Description() const override { return "Melee hits make people and cars explode"; }
    int MaxLevel() const override { return kLevels; }

    void LoadConfig(const Config& config, const char* section) override;
    void OnActivate(CPlayerPed* player) override;
    void Update(CPlayerPed* player, float dt) override;
    void OnDeactivate(CPlayerPed* player) override;

private:
    void Explode(CPlayerPed* player, CEntity* victim);

    // per level (index 0 = level 1)
    int m_ringBlasts[kLevels] = {3, 6, 10};           // extra explosions around the victim
    float m_ringRadius[kLevels] = {5.0f, 9.0f, 14.0f}; // m
    float m_shockRadius[kLevels] = {18.0f, 30.0f, 45.0f};
    float m_shockSpeed[kLevels] = {30.0f, 50.0f, 80.0f}; // m/s at the center
    float m_launch[kLevels] = {40.0f, 65.0f, 100.0f};    // m/s: the victim flies away from CJ

    float m_radius = 6.0f;      // only things this close to CJ are checked (a punch is ~1-2 m)
    float m_cooldown = 0.25f;   // seconds between explosions, so one swing = one blast
    float m_cooldownLeft = 0.0f;
    bool m_wasExplosionProof = false;

    // health of each nearby ped/vehicle last frame, to spot drops
    std::unordered_map<CEntity*, float> m_lastHealth;
};
