#pragma once

#include "core/Power.h"

#include <unordered_map>

class CAnimBlendAssociation;

// Phase 3.7: Bullet Time. The world slows down; CJ keeps moving (almost) at normal speed.
// 3 levels = how slow: 60% / 35% / 15% speed.
//
// How: CTimer::ms_fTimeScale slows the whole game (physics, AI, animations, bullets). To keep CJ
// fast, every animation playing on him runs 1/scale times faster. Since peds move by their
// animations, that brings his walking/running/fighting back to normal speed. Things driven by
// physics (his jumps and falls) stay slow-motion, which looks cool anyway.
class BulletTime : public Power {
public:
    static constexpr int kLevels = 3;

    const char* Name() const override { return "Bullet Time"; }
    const char* Id() const override { return "BulletTime"; }
    const char* Description() const override { return "The world slows down, CJ keeps moving at normal speed"; }
    int MaxLevel() const override { return kLevels; }

    void LoadConfig(const Config& config, const char* section) override;
    void OnActivate(CPlayerPed* player) override;
    void Update(CPlayerPed* player, float dt) override;
    void OnDeactivate(CPlayerPed* player) override;
    void Draw() override;

private:
    void SpeedUpPlayerAnims(CPlayerPed* player, float factor);
    void RestorePlayerAnims(CPlayerPed* player);

    float m_scale[kLevels] = {0.60f, 0.35f, 0.15f}; // game speed (1 = normal)
    bool m_playerNormalSpeed = true;
    int m_tintAlpha = 35; // 0 = no screen tint

    // per animation on CJ: its own speed (base) and the speed we set (base / scale).
    // If the game changes an anim's speed itself, we notice (speed != what we set) and take that as the new base.
    struct AnimSpeed {
        float base;
        float applied;
    };
    std::unordered_map<CAnimBlendAssociation*, AnimSpeed> m_anims;
};
