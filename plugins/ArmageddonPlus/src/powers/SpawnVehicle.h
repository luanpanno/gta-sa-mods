#pragma once

#include "core/Power.h"

#include <string>
#include <vector>

// Vehicle spawner with a picker of EVERY spawnable vehicle in the game (cars, bikes, boats,
// planes, helicopters, trailers...). In the menu: Left/Right choose (Shift = 10 at a time), Enter spawns.
//
// The list is built from the game's own data: every model 400-611 whose model info is a vehicle
// (minus trains and RC toys), named with the game's text for it (TheText, e.g. "Infernus").
// Spawning uses engine classes directly (CStreaming, CModelInfo, CAutomobile/CBike/..., CWorld).
class SpawnVehicle : public Power {
public:
    const char* Name() const override { return "Spawn Vehicle"; }
    const char* Id() const override { return "SpawnVehicle"; }
    const char* Description() const override { return "Left/Right choose a vehicle (Shift: 10 at a time), Enter spawns it in front of CJ"; }
    const char* Category() const override { return "Spawn"; }
    Mode GetMode() const override { return Mode::OneShot; }

    bool HasPicker() const override { return true; }
    const char* PickerValue() const override;
    void PickerStep(int delta) override;

    void LoadConfig(const Config& config, const char* section) override;
    void OnActivate(CPlayerPed* player) override;

private:
    void BuildList() const;

    int m_startModel = 411; // Infernus, or Model= in the ini
    mutable std::vector<int> m_models;         // spawnable model IDs
    mutable std::vector<std::string> m_names;  // their display names
    mutable int m_index = 0;
};
