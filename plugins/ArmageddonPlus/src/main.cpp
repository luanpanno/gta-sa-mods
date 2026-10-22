// ArmageddonPlus: entry point.
// Phase 2.1: hello world. Proves the toolchain works: build -> deploy -> loaded by the game.
//
//   - draws "ArmageddonPlus" in the top-right corner of the HUD
//   - F10 shows a help box with live data read from the engine (CJ's health, position)
//   - writes ArmageddonPlus.log in the game folder

#include "plugin.h"
#include "CFont.h"
#include "CHud.h"
#include "CPlayerPed.h"

#include <cstdio>
#include <fstream>

using namespace plugin;

namespace {

std::ofstream g_log;

// KeyPressed() is true for every frame the key is held; this turns it into "just pressed".
bool KeyJustPressed(unsigned int vk) {
    static bool wasDown[256] = {};
    bool down = KeyPressed(vk);
    bool justPressed = down && !wasDown[vk];
    wasDown[vk] = down;
    return justPressed;
}

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

void OnGameTick() {
    CPlayerPed* player = FindPlayerPed();
    if (!player)
        return;

    if (KeyJustPressed(VK_F10)) {
        CVector pos = player->GetPosition();
        static char text[200];
        std::snprintf(text, sizeof(text),
                      "Hello from C++!~n~Health: ~y~%.0f~s~~n~Position: ~y~%.1f, %.1f, %.1f",
                      player->m_fHealth, pos.x, pos.y, pos.z);
        CHud::SetHelpMessage(text, true, false, false);
        g_log << "F10 pressed at " << pos.x << ", " << pos.y << ", " << pos.z << std::endl;
    }
}

} // namespace

class ArmageddonPlus {
public:
    ArmageddonPlus() {
        // runs once, when the ASI loader loads us (before the game starts)
        g_log.open("ArmageddonPlus.log", std::ios::out | std::ios::trunc);
        g_log << "ArmageddonPlus loaded" << std::endl;

        Events::gameProcessEvent += OnGameTick; // every game frame (logic)
        Events::drawHudEvent += DrawWatermark;  // every frame, while the HUD is drawn
    }
} g_armageddonPlus; // global instance: its constructor is our "main"
