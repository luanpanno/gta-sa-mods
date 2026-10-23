#include "world/Chaos.h"

#include "core/Config.h"
#include "core/Log.h"
#include "core/Sound.h"

#include "plugin.h"
#include "CCheat.h"
#include "CExplosion.h"
#include "CPlayerPed.h"
#include "CPools.h"
#include "CVehicle.h"
#include "CWorld.h"

#include <cmath>
#include <cstdlib>
#include <vector>

namespace {

const float kStepsPerSecond = 50.0f; // m_vecMoveSpeed is per 1/50 s step

float RandomRange(float lo, float hi) {
    return lo + (hi - lo) * ((float)std::rand() / RAND_MAX);
}

} // namespace

const char* Chaos::LevelName(int level) const {
    static const char* kNames[kLevels] = {"Meteors", "Exploding Traffic", "Flying Cars", "Riot"};
    return level >= 1 && level <= kLevels ? kNames[level - 1] : nullptr;
}

void Chaos::LoadConfig(const Config& config, const char* section) {
    m_interval[0] = config.GetFloat(section, "MeteorInterval", m_interval[0]);
    m_interval[1] = config.GetFloat(section, "ExplodeInterval", m_interval[1]);
    m_interval[2] = config.GetFloat(section, "LaunchInterval", m_interval[2]);
    m_radius = config.GetFloat(section, "Radius", m_radius);
}

void Chaos::SetRiot(bool on) {
    if (on == m_riotOn)
        return;
    if (on)
        m_riotWasOn = CCheat::m_aCheatsActive[CHEAT_RIOT_MODE]; // the player may have typed the cheat
    CCheat::m_aCheatsActive[CHEAT_RIOT_MODE] = on ? true : m_riotWasOn;
    m_riotOn = on;
}

void Chaos::OnLevelChanged(CPlayerPed*, int level) {
    SetRiot(level == 4);
    m_timer = 0.0f;
}

CVehicle* Chaos::RandomVehicle(CPlayerPed* player, float radius) {
    std::vector<CVehicle*> candidates;
    CVector center = player->GetPosition();
    auto* vehicles = CPools::ms_pVehiclePool;
    for (int i = 0; i < vehicles->m_nSize; ++i) {
        CVehicle* vehicle = vehicles->GetAt(i);
        if (!vehicle || (player->bInVehicle && vehicle == player->m_pVehicle))
            continue;
        if (vehicle->m_nCreatedBy == eVehicleCreatedBy::MISSION_VEHICLE)
            continue; // never break missions
        if (vehicle->m_nStatus == STATUS_WRECKED)
            continue;
        if ((vehicle->GetPosition() - center).Magnitude() > radius)
            continue;
        candidates.push_back(vehicle);
    }
    if (candidates.empty())
        return nullptr;
    return candidates[std::rand() % candidates.size()];
}

void Chaos::Meteor(CPlayerPed* player) {
    // a random spot 12..radius m away (never right on top of CJ), on the ground there
    float angle = RandomRange(0.0f, 6.2832f);
    float distance = RandomRange(12.0f, m_radius);
    CVector center = player->GetPosition();
    float x = center.x + std::cos(angle) * distance;
    float y = center.y + std::sin(angle) * distance;
    float z = CWorld::FindGroundZForCoord(x, y);
    CExplosion::AddExplosion(nullptr, nullptr, EXPLOSION_ROCKET, CVector(x, y, z), 0, true, 0.3f, true);
}

void Chaos::Update(CPlayerPed* player, float dt) {
    int level = Level();
    if (level == 4)
        return; // riot: the game does the work (SetRiot)

    m_timer -= dt;
    if (m_timer > 0.0f)
        return;
    m_timer = m_interval[level - 1];

    switch (level) {
    case 1:
        Meteor(player);
        break;
    case 2:
        if (CVehicle* vehicle = RandomVehicle(player, m_radius))
            vehicle->BlowUpCar(nullptr, false);
        break;
    case 3:
        if (CVehicle* vehicle = RandomVehicle(player, m_radius)) {
            vehicle->SetIsStatic(false); // parked cars sleep until woken
            vehicle->m_vecMoveSpeed.z += RandomRange(10.0f, 18.0f) / kStepsPerSecond;
            vehicle->m_vecTurnSpeed += CVector(RandomRange(-0.05f, 0.05f), RandomRange(-0.05f, 0.05f), RandomRange(-0.1f, 0.1f));
            Sound::Play(Sound::Whoosh, 0.4f);
        }
        break;
    }
}

void Chaos::OnDeactivate(CPlayerPed*) {
    SetRiot(false);
}
