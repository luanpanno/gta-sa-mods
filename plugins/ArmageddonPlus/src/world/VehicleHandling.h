#pragma once

#include "core/Power.h"

class CVehicle;

// Phase 5.2: super handling for whatever CJ drives. Levels: Sport / Race / Rocket.
//
//   hold SHIFT   turbo: extra acceleration, up to a higher top speed
//   J            jump (when the wheels are on the ground)
//   always       extra grip (less sliding in corners), tyres can't burst,
//                auto-flip back onto the wheels if the car lands upside down
//
// How: like the powers, it's all velocity math on the car's CPhysical: add along GetForward() for
// turbo, remove the sideways part (GetRight()) for grip, add upward for the jump. Tyres use the
// CVehicle::bTyresDontBurst flag (saved and restored per car).
class VehicleHandling : public Power {
public:
    static constexpr int kLevels = 3;

    const char* Name() const override { return "Vehicle Handling"; }
    const char* Id() const override { return "VehicleHandling"; }
    const char* Category() const override { return "World"; }
    int MaxLevel() const override { return kLevels; }
    const char* LevelName(int level) const override;

    void LoadConfig(const Config& config, const char* section) override;
    void Update(CPlayerPed* player, float dt) override;
    void OnDeactivate(CPlayerPed* player) override;

private:
    void TrackVehicle(CVehicle* vehicle); // tyres flag on the current car, restored on the previous one

    float m_turboAccel[kLevels] = {12.0f, 25.0f, 50.0f}; // m/s per second
    float m_topSpeed[kLevels] = {55.0f, 85.0f, 140.0f};  // m/s with turbo
    float m_grip[kLevels] = {2.0f, 4.0f, 8.0f};          // how fast sideways sliding is removed (1/s)
    float m_jump[kLevels] = {6.0f, 9.0f, 14.0f};         // m/s upward
    int m_turboKey = 0;
    int m_jumpKey = 0;

    CVehicle* m_vehicle = nullptr;
    bool m_vehicleTyresWereProof = false;
    float m_upsideDownFor = 0.0f; // seconds the car has been upside down and stopped
    bool m_turboWasDown = false;
};
