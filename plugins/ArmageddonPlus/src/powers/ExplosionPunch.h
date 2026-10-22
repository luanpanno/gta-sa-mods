#pragma once

#include "core/Power.h"

#include <unordered_map>

class CEntity;

// Phase 3.3: CJ's melee hits (fists, bat, katana...) make the victim explode. 3 levels:
// 1 grenade-size, 2 rocket-size, 3 car-bomb-size with camera shake.
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
    int DefaultKey() const override;
    int MaxLevel() const override { return kLevels; }

    void LoadConfig(const Config& config, const char* section) override;
    void OnActivate(CPlayerPed* player) override;
    void Update(CPlayerPed* player, float dt) override;
    void OnDeactivate(CPlayerPed* player) override;

private:
    void Explode(CPlayerPed* player, CEntity* victim);

    float m_radius = 6.0f;      // only things this close to CJ are checked (a punch is ~1-2 m)
    float m_cooldown = 0.25f;   // seconds between explosions, so one swing = one blast
    float m_cooldownLeft = 0.0f;
    bool m_wasExplosionProof = false;

    // health of each nearby ped/vehicle last frame, to spot drops
    std::unordered_map<CEntity*, float> m_lastHealth;
};
