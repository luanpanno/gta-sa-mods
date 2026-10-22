#pragma once

#include "core/Power.h"

// Phase 2.2: the CLEO vehicle spawner, ported to engine classes (CStreaming, CModelInfo,
// CAutomobile/CBike/..., CWorld). One-shot: press the key, a vehicle appears in front of CJ.
class SpawnVehicle : public Power {
public:
    const char* Name() const override { return "Spawn Vehicle"; }
    const char* Id() const override { return "SpawnVehicle"; }
    Mode GetMode() const override { return Mode::OneShot; }

    void LoadConfig(const Config& config, const char* section) override;
    void OnActivate(CPlayerPed* player) override;

private:
    int m_model = 411; // Infernus
};
