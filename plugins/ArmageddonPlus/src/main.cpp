// ArmageddonPlus: entry point.
// Wires the core framework (config, input, power manager, menu) to the game's events and registers the powers.

#include "core/Config.h"
#include "core/Energy.h"
#include "core/Fx.h"
#include "core/Input.h"
#include "core/Sound.h"
#include "core/Log.h"
#include "core/Menu.h"
#include "core/PowerManager.h"
#include "powers/BulletTime.h"
#include "powers/ExplosionPunch.h"
#include "powers/FireTrail.h"
#include "powers/LightningStrike.h"
#include "powers/Flight.h"
#include "powers/Force.h"
#include "powers/Invulnerability.h"
#include "powers/SpawnVehicle.h"
#include "powers/SuperSpeed.h"
#include "powers/Telekinesis.h"

#include "plugin.h"
#include "CFont.h"
#include "CTimer.h"

#include <memory>

using namespace plugin;

namespace {

PowerManager g_powers;
Menu g_menu(g_powers);

void DrawWatermark() {
    CFont::SetOrientation(ALIGN_RIGHT);
    CFont::SetRightJustifyWrap(0.0f);
    CFont::SetFontStyle(FONT_SUBTITLES);
    CFont::SetProportional(true);
    CFont::SetBackground(false, false);
    CFont::SetScale(SCREEN_MULTIPLIER(0.5f), SCREEN_MULTIPLIER(1.0f));
    CFont::SetColor(CRGBA(255, 160, 0, 255));
    CFont::SetDropShadowPosition(1);
    CFont::SetDropColor(CRGBA(0, 0, 0, 255));
    CFont::PrintString(SCREEN_COORD_RIGHT(20.0f), SCREEN_COORD_TOP(10.0f), "ArmageddonPlus");
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
        g_powers.Register(std::make_unique<ExplosionPunch>());
        g_powers.Register(std::make_unique<Telekinesis>());
        g_powers.Register(std::make_unique<Force>());
        g_powers.Register(std::make_unique<LightningStrike>());
        g_powers.Register(std::make_unique<FireTrail>());
        g_powers.Register(std::make_unique<BulletTime>()); // after the others: it speeds up CJ's anims as they are
        g_powers.Register(std::make_unique<Invulnerability>());
        g_powers.Register(std::make_unique<SpawnVehicle>());

        Config config;
        config.Load(paths::GetPluginDirRelativePathA("ArmageddonPlus.ini"));
        Fx::LoadConfig(config);
        Sound::LoadConfig(config); // synthesizes the sounds (a few ms)
        g_powers.LoadConfig(config);
        g_menu.LoadConfig(config);

        Events::gameProcessEvent += [] { // every game frame (logic)
            Input::Update();
            g_menu.Update(CTimer::ms_fTimeStep / 50.0f); // before powers: it pauses their hotkeys
            g_powers.Update();
        };
        Events::drawHudEvent += [] { // every frame, while the HUD is drawn
            g_powers.DrawPowers(); // screen effects first, so text draws on top
            DrawWatermark();
            Energy::Draw(SCREEN_COORD_RIGHT(20.0f), SCREEN_COORD_TOP(36.0f)); // under the watermark
            if (!g_menu.IsOpen())
                g_powers.DrawOverlay(); // the menu already shows every power's state
            g_menu.Draw();
        };
    }
} g_armageddonPlus; // global instance: its constructor is our "main"
