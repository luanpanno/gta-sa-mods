#include "powers/LightningStrike.h"

#include "core/Config.h"
#include "core/Shockwave.h"
#include "core/Sound.h"
#include "core/Input.h"
#include "core/Log.h"

#include "plugin.h"
#include "CCamera.h"
#include "CColPoint.h"
#include "CExplosion.h"
#include "CFireManager.h"
#include "CPad.h"
#include "CPlayerPed.h"
#include "CRect.h"
#include "CSprite.h"
#include "CSprite2d.h"
#include "CWorld.h"

#include <windows.h>

#include <cmath>
#include <cstdio>
#include <cstdlib>

using namespace plugin;

namespace {

const float kBoltSeconds = 0.25f;  // how long the bolt stays on screen
const float kFlashSeconds = 0.15f; // how long the white flash takes to fade
const float kBoltHeight = 90.0f;   // m above the target where the bolt starts

// main bolt hits the target; extra bolts land around it
struct LevelInfo {
    eExplosionType main;
    eExplosionType extra;
    float camShake;
    bool fire;
};
const LevelInfo kLevelInfo[LightningStrike::kLevels] = {
    {EXPLOSION_CAR, EXPLOSION_ROCKET, 0.6f, false},
    {EXPLOSION_CAR, EXPLOSION_QUICK_CAR, 1.0f, true},
    {EXPLOSION_AIRCRAFT, EXPLOSION_CAR, 1.6f, true},
};

unsigned long long NowMs() {
    return GetTickCount64(); // wall clock: Bullet Time doesn't slow the bolt animation
}

float Random(float range) {
    return ((float)std::rand() / RAND_MAX * 2.0f - 1.0f) * range;
}

bool ToScreen(const CVector& world, float& x, float& y) {
    RwV3d in{world.x, world.y, world.z};
    RwV3d out;
    float w, h;
    if (!CSprite::CalcScreenCoors(in, &out, &w, &h, false) || out.z <= 1.0f)
        return false; // behind the camera
    x = out.x; // screen pixels
    y = out.y;
    return true;
}

void Segment(float x1, float y1, float x2, float y2, float width, CRGBA color) {
    // a line as a thin quad: offset both ends sideways by half the width
    float dx = x2 - x1, dy = y2 - y1;
    float length = std::sqrt(dx * dx + dy * dy);
    if (length < 0.001f)
        return;
    float nx = -dy / length * width * 0.5f, ny = dx / length * width * 0.5f;
    CSprite2d::Draw2DPolygon(x1 + nx, y1 + ny, x1 - nx, y1 - ny, x2 + nx, y2 + ny, x2 - nx, y2 - ny, color);
}

} // namespace

void LightningStrike::LoadConfig(const Config& config, const char* section) {
    char key[32];
    for (int i = 0; i < kLevels; ++i) {
        std::snprintf(key, sizeof(key), "Bolts%d", i + 1);
        m_boltCount[i] = config.GetInt(section, key, m_boltCount[i]);
        std::snprintf(key, sizeof(key), "Spread%d", i + 1);
        m_spread[i] = config.GetFloat(section, key, m_spread[i]);
        std::snprintf(key, sizeof(key), "ShockRadius%d", i + 1);
        m_shockRadius[i] = config.GetFloat(section, key, m_shockRadius[i]);
        std::snprintf(key, sizeof(key), "ShockSpeed%d", i + 1);
        m_shockSpeed[i] = config.GetFloat(section, key, m_shockSpeed[i]);
    }
    m_range = config.GetFloat(section, "Range", m_range);
    m_cooldown = config.GetFloat(section, "Cooldown", m_cooldown);
}

void LightningStrike::OnActivate(CPlayerPed* player) {
    if (CPad* pad = CPad::GetPad(0))
        m_hadFireDisabled = pad->bDisablePlayerFireWeapon;
    m_wasExplosionProof = player->bExplosionProof;
    m_bolts.clear();
}

void LightningStrike::AddBolt(const CVector& target) {
    // a jagged line from the sky down to the target; the zig-zag shrinks near the ground
    Bolt bolt;
    float lean = Random(10.0f); // each bolt comes in at a slightly different angle
    for (int i = 0; i < kBoltPoints; ++i) {
        float t = (float)i / (kBoltPoints - 1); // 0 = sky, 1 = target
        float jitter = (i == 0 || i == kBoltPoints - 1) ? 0.0f : 6.0f * (1.0f - t * 0.7f);
        bolt.points[i] = CVector(target.x + Random(jitter) + (1.0f - t) * lean, target.y + Random(jitter),
                                 target.z + (1.0f - t) * kBoltHeight);
    }
    m_bolts.push_back(bolt);
}

void LightningStrike::Strike(CPlayerPed* player) {
    const CCam& cam = TheCamera.m_aCams[TheCamera.m_nActiveCam];
    CVector from = cam.m_vecSource;
    CVector to = from + cam.m_vecFront * m_range;

    CColPoint hit;
    CEntity* entity = nullptr;
    CWorld::pIgnoreEntity = player;
    bool found = CWorld::ProcessLineOfSight(from, to, hit, entity, true, true, true, true, false, false, false, false);
    CWorld::pIgnoreEntity = nullptr;
    if (!found)
        return; // pointing at the sky
    if (!PayForUse())
        return; // not enough energy

    int lvl = Level() - 1;
    CVector target = hit.m_vecPoint;
    const LevelInfo& info = kLevelInfo[lvl];
    m_bolts.clear();

    // main bolt
    AddBolt(target);
    CExplosion::AddExplosion(nullptr, player, info.main, target, 0, true, info.camShake, true);
    if (info.fire)
        gFireManager.StartFire(target, 1.0f, 1, player, 8000, 1, 1);

    // extra bolts scattered around the target (staggered so it rumbles like a storm)
    for (int i = 1; i < m_boltCount[lvl]; ++i) {
        float angle = Random(3.1416f);
        float distance = m_spread[lvl] * (0.4f + 0.6f * ((float)std::rand() / RAND_MAX));
        CVector point(target.x + std::cos(angle) * distance, target.y + std::sin(angle) * distance, target.z);
        point.z = CWorld::FindGroundZForCoord(point.x, point.y);
        AddBolt(point);
        CExplosion::AddExplosion(nullptr, player, info.extra, point, 80 * i, true, 0.0f, true);
        if (info.fire)
            gFireManager.StartFire(point, 1.0f, 1, player, 6000, 1, 1);
    }

    // shockwave from the impact: everything around gets thrown
    Shockwave::Params params;
    params.radius = m_shockRadius[lvl];
    params.speed = m_shockSpeed[lvl];
    params.lift = 0.5f;
    Shockwave::Blast(target, params, player);

    m_lastStrikeMs = NowMs();
    Sound::Play(Sound::Thunder, 0.6f + 0.2f * (Level() - 1));
    Log::Info("Lightning strike (level %d)", Level());
}

void LightningStrike::Update(CPlayerPed* player, float) {
    if (player->bInVehicle)
        return;
    if (CPad* pad = CPad::GetPad(0))
        pad->bDisablePlayerFireWeapon = 1; // LMB strikes instead of punching/shooting
    player->bExplosionProof = true;        // striking close to yourself shouldn't kill you

    float sinceLast = (NowMs() - m_lastStrikeMs) / 1000.0f;
    if (Input::JustPressed(VK_LBUTTON) && sinceLast >= m_cooldown)
        Strike(player);
}

void LightningStrike::OnDeactivate(CPlayerPed* player) {
    if (CPad* pad = CPad::GetPad(0))
        pad->bDisablePlayerFireWeapon = m_hadFireDisabled;
    player->bExplosionProof = m_wasExplosionProof;
}

void LightningStrike::DrawBolt(const Bolt& bolt, float age) {
    float fade = 1.0f - age / kBoltSeconds;
    float sx[kBoltPoints], sy[kBoltPoints];
    bool visible[kBoltPoints];
    for (int i = 0; i < kBoltPoints; ++i)
        visible[i] = ToScreen(bolt.points[i], sx[i], sy[i]);

    // two passes: a wide blue glow, then a thin white core
    for (int pass = 0; pass < 2; ++pass) {
        float width = pass == 0 ? SCREEN_COORD(12.0f) : SCREEN_COORD(3.5f);
        CRGBA color = pass == 0 ? CRGBA(120, 160, 255, (unsigned char)(120 * fade))
                                : CRGBA(255, 255, 255, (unsigned char)(255 * fade));
        for (int i = 0; i + 1 < kBoltPoints; ++i)
            if (visible[i] && visible[i + 1])
                Segment(sx[i], sy[i], sx[i + 1], sy[i + 1], width, color);
    }
}
void LightningStrike::DrawCrosshair() {
    float cx = SCREEN_WIDTH / 2.0f, cy = SCREEN_HEIGHT / 2.0f;
    float arm = SCREEN_COORD(7.0f), gap = SCREEN_COORD(3.0f), thick = SCREEN_COORD(1.5f);
    CRGBA color(180, 210, 255, 220);
    CSprite2d::DrawRect(CRect(cx - arm - gap, cy - thick, cx - gap, cy + thick), color); // left
    CSprite2d::DrawRect(CRect(cx + gap, cy - thick, cx + arm + gap, cy + thick), color); // right
    CSprite2d::DrawRect(CRect(cx - thick, cy - arm - gap, cx + thick, cy - gap), color); // top
    CSprite2d::DrawRect(CRect(cx - thick, cy + gap, cx + thick, cy + arm + gap), color); // bottom
}

void LightningStrike::Draw() {
    DrawCrosshair();
    if (m_bolts.empty())
        return;
    float age = (NowMs() - m_lastStrikeMs) / 1000.0f;
    if (age < kFlashSeconds) {
        unsigned char alpha = (unsigned char)(140 * (1.0f - age / kFlashSeconds));
        CSprite2d::DrawRect(CRect(0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT), CRGBA(230, 235, 255, alpha));
    }
    if (age < kBoltSeconds) {
        for (const Bolt& bolt : m_bolts)
            DrawBolt(bolt, age);
    } else {
        m_bolts.clear();
    }
}
