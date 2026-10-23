#pragma once

class Config;

// Phase 4.2: one shared energy pool for all powers (can be switched off in the menu).
//
//   - toggle powers drain it every second while they're on (Drain1..3 in their ini section);
//     at 0 they switch off
//   - actions cost a lump sum (Cost1..3): an explosion punch, a lightning strike, a force blast...
//   - it refills by itself after a short pause without spending (RegenDelay, Regen per second)
//
// All times are REAL seconds (Bullet Time doesn't make powers cheaper or recharge slower).
namespace Energy {

void LoadConfig(const Config& config);

bool IsEnabled();          // false = infinite energy, no bar
void SetEnabled(bool enabled);

float Current();
float Max();

void Update(float realDt);              // regen; call once per frame
bool TrySpend(float amount);            // pay for an action; false (+ error feedback) if not enough
bool Drain(float amount);               // continuous drain; false when empty
bool CanStart();                        // enough to switch a power on

bool IsFailFlashing();                  // true briefly after a failed spend (the HUD bar flashes red)

} // namespace Energy
