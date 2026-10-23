#include "world/Reactions.h"

#include "core/Config.h"
#include "core/PowerManager.h"

#include "plugin.h"
#include "CPlayerPed.h"
#include "CPools.h"
#include "CWanted.h"
#include "extensions/ScriptCommands.h"

using namespace plugin;

namespace {
const unsigned char kCreatedByMission = 2; // CPed::m_nCreatedBy: 1 = random ped, 2 = mission ped
const float kRefleeAfter = 10.0f;          // seconds before the same ped can be told to flee again
}

const char* Reactions::LevelName(int level) const {
    static const char* kNames[kLevels] = {"Fear", "Fear + Police"};
    return level >= 1 && level <= kLevels ? kNames[level - 1] : nullptr;
}

void Reactions::LoadConfig(const Config& config, const char* section) {
    m_radius = config.GetFloat(section, "Radius", m_radius);
    m_policeLevel = config.GetInt(section, "PoliceStars", m_policeLevel);
}

void Reactions::Update(CPlayerPed* player, float dt) {
    m_clock += dt;
    m_timer -= dt;
    if (m_timer > 0.0f)
        return;
    m_timer = 0.75f; // a few times per second is plenty

    if (!PowerManager::Instance().AnyActive("Powers"))
        return; // no superpower on: nothing to react to

    // forget peds that are gone or calmed down
    for (auto it = m_fledAt.begin(); it != m_fledAt.end();) {
        if (m_clock - it->second > kRefleeAfter || !CPools::ms_pPedPool->IsObjectValid(it->first))
            it = m_fledAt.erase(it);
        else
            ++it;
    }

    int cj = CPools::GetPedRef(player);
    CVector center = player->GetPosition();
    auto* peds = CPools::ms_pPedPool;
    for (int i = 0; i < peds->m_nSize; ++i) {
        CPed* ped = peds->GetAt(i);
        if (!ped || ped == player || ped->bInVehicle || ped->m_fHealth <= 0.0f)
            continue;
        if (ped->m_nCreatedBy == kCreatedByMission || m_fledAt.count(ped))
            continue;
        if ((ped->GetPosition() - center).Magnitude() > m_radius)
            continue;
        // run (up to 60 m) away from CJ for 8 seconds
        Command<Commands::TASK_SMART_FLEE_CHAR>(CPools::GetPedRef(ped), cj, 60.0f, 8000);
        m_fledAt[ped] = m_clock;
    }

    if (Level() == 2) {
        CWanted* wanted = player->GetWanted();
        if (wanted && (int)wanted->m_nWantedLevel < m_policeLevel)
            Command<Commands::ALTER_WANTED_LEVEL_NO_DROP>(0, m_policeLevel);
    }
}

void Reactions::OnDeactivate(CPlayerPed*) {
    m_fledAt.clear();
}
