// ArmageddonPlus: entry point.
// Wires the core framework (config, input, power manager, menu) to the game's events and registers the powers.

#include "core/Config.h"
#include "core/Energy.h"
#include "core/FallGuard.h"
#include "core/Fx.h"
#include "core/Hud.h"
#include "core/Input.h"
#include "core/Leap.h"
#include "core/Ui.h"
#include "core/UserSettings.h"
#include "core/Sound.h"
#include "core/Log.h"
#include "core/Menu.h"
#include "core/PowerManager.h"
#include "powers/AddAmmo.h"
#include "powers/AddMoney.h"
#include "powers/BulletTime.h"
#include "powers/GiveWeapon.h"
#include "powers/ExplosionPunch.h"
#include "powers/FireTrail.h"
#include "powers/LightningStrike.h"
#include "powers/Flight.h"
#include "powers/Force.h"
#include "powers/Invulnerability.h"
#include "powers/SpawnVehicle.h"
#include "powers/SuperJump.h"
#include "powers/SuperSpeed.h"
#include "powers/Telekinesis.h"
#include "world/Chaos.h"
#include "world/Gravity.h"
#include "world/Reactions.h"
#include "world/VehicleHandling.h"

#include "plugin.h"
#include "CFont.h"
#include "CTimer.h"

#include <memory>

using namespace plugin;

namespace {

PowerManager g_powers;
Menu g_menu(g_powers);

// real seconds this frame (the menu shouldn't slow down in Bullet Time)
float RealDt() {
    float timeScale = CTimer::ms_fTimeScale > 0.05f ? CTimer::ms_fTimeScale : 0.05f;
    return CTimer::ms_fTimeStep / 50.0f / timeScale;
}

} // namespace

class ArmageddonPlus {
public:
    ArmageddonPlus() {
        // runs once, when the ASI loader loads us (before the game starts)
        Log::Init(paths::GetGameDirRelativePathA("ArmageddonPlus.log"));
        Log::Info("ArmageddonPlus loaded from %s", paths::GetPluginDirPathA());

        // register powers here (order = order in the overlay)
        g_powers.Register(std::make_unique<Flight>());
        g_powers.Register(std::make_unique<SuperSpeed>());
        g_powers.Register(std::make_unique<SuperJump>()); // after Super Speed: its jump wins when both are on
        g_powers.Register(std::make_unique<ExplosionPunch>());
        g_powers.Register(std::make_unique<Telekinesis>());
        g_powers.Register(std::make_unique<Force>());
        g_powers.Register(std::make_unique<LightningStrike>());
        g_powers.Register(std::make_unique<FireTrail>());
        g_powers.Register(std::make_unique<BulletTime>()); // after the others: it speeds up CJ's anims as they are
        g_powers.Register(std::make_unique<Invulnerability>());
        // spawning tools (own "Spawn" section in the menu): Left/Right pick, Enter use
        g_powers.Register(std::make_unique<SpawnVehicle>());
        g_powers.Register(std::make_unique<GiveWeapon>());
        g_powers.Register(std::make_unique<AddAmmo>());
        g_powers.Register(std::make_unique<AddMoney>());
        // world mods (own "World" section in the menu)
        g_powers.Register(std::make_unique<Gravity>());
        g_powers.Register(std::make_unique<VehicleHandling>());
        g_powers.Register(std::make_unique<Chaos>());
        g_powers.Register(std::make_unique<Reactions>());

        Config config;
        config.Load(paths::GetPluginDirRelativePathA("ArmageddonPlus.ini"));
        Ui::LoadConfig(config);
        Fx::LoadConfig(config);
        Sound::LoadConfig(config); // synthesizes the sounds (a few ms)
        g_powers.LoadConfig(config);
        g_menu.LoadConfig(config);
        UserSettings::Apply(g_powers); // your shortcuts and toggles (ArmageddonPlus.user.ini) win

        Events::gameProcessEvent += [] { // every game frame (logic)
            Input::Update();
            g_menu.Update(RealDt()); // before powers: it pauses their shortcuts
            g_powers.Update();
            Leap::Update(FindPlayerPed());      // drives big jump arcs
            FallGuard::Update(FindPlayerPed()); // ends jump fall-protection on landing
        };
        Events::drawHudEvent += [] { // every frame, while the HUD is drawn
            g_powers.DrawPowers(); // screen effects first, so the HUD draws on top
            if (!g_menu.IsOpen())
                Hud::Draw(g_powers); // the menu already shows everything
            Hud::DrawEnergy();       // bottom center, also while the menu is open
            g_menu.Draw();
        };
    }
} g_armageddonPlus; // global instance: its constructor is our "main"
