#include "powers/BulletTime.h"

#include "core/Config.h"
#include "core/Sound.h"

#include "plugin.h"
#include "common.h" // RpAnimBlendClumpGetFirstAssociation / RpAnimBlendGetNextAssociation
#include "CAnimBlendAssociation.h"
#include "CPlayerPed.h"
#include "CRect.h"
#include "CSprite2d.h"
#include "CTimer.h"

#include <cmath>
#include <cstdio>

using namespace plugin;

void BulletTime::LoadConfig(const Config& config, const char* section) {
    char key[32];
    for (int i = 0; i < kLevels; ++i) {
        std::snprintf(key, sizeof(key), "Scale%d", i + 1);
        m_scale[i] = config.GetFloat(section, key, m_scale[i]);
        if (m_scale[i] < 0.05f)
            m_scale[i] = 0.05f; // below this physics gets unstable
    }
    m_playerNormalSpeed = config.GetBool(section, "PlayerNormalSpeed", true);
    m_tintAlpha = config.GetInt(section, "TintAlpha", m_tintAlpha);
}

void BulletTime::SpeedUpPlayerAnims(CPlayerPed* player, float factor) {
    if (!player->m_pRwClump)
        return;
    std::unordered_map<CAnimBlendAssociation*, AnimSpeed> seen;
    for (auto* anim = RpAnimBlendClumpGetFirstAssociation(player->m_pRwClump); anim;
         anim = RpAnimBlendGetNextAssociation(anim)) {
        AnimSpeed speed;
        auto known = m_anims.find(anim);
        if (known != m_anims.end() && std::fabs(anim->m_fSpeed - known->second.applied) < 0.0001f)
            speed.base = known->second.base; // still the value we set last frame
        else
            speed.base = anim->m_fSpeed;     // new anim, or the game changed its speed
        speed.applied = speed.base * factor;
        anim->m_fSpeed = speed.applied;
        seen[anim] = speed;
    }
    m_anims = std::move(seen); // forget anims that finished
}

void BulletTime::RestorePlayerAnims(CPlayerPed* player) {
    if (player && player->m_pRwClump) {
        for (auto* anim = RpAnimBlendClumpGetFirstAssociation(player->m_pRwClump); anim;
             anim = RpAnimBlendGetNextAssociation(anim)) {
            auto known = m_anims.find(anim);
            if (known != m_anims.end() && std::fabs(anim->m_fSpeed - known->second.applied) < 0.0001f)
                anim->m_fSpeed = known->second.base;
        }
    }
    m_anims.clear();
}

void BulletTime::Update(CPlayerPed* player, float) {
    float scale = m_scale[Level() - 1];
    CTimer::ms_fTimeScale = scale; // re-applied every frame (the game resets it, e.g. after menus)

    if (m_playerNormalSpeed && !player->bInVehicle)
        SpeedUpPlayerAnims(player, 1.0f / scale);
    else
        RestorePlayerAnims(player);
}

void BulletTime::OnActivate(CPlayerPed*) {
    Sound::Play(Sound::SlowDown);
}

void BulletTime::OnDeactivate(CPlayerPed* player) {
    if (player->m_fHealth > 0.0f)
        Sound::Play(Sound::SpeedUp);
    CTimer::ms_fTimeScale = 1.0f;
    RestorePlayerAnims(player);
}

void BulletTime::Draw() {
    if (m_tintAlpha <= 0)
        return;
    // slight cold tint over the screen while time is slowed
    CSprite2d::DrawRect(CRect(0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT), CRGBA(60, 110, 160, (unsigned char)m_tintAlpha));
}
