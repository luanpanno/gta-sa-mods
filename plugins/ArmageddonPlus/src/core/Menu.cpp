#include "core/Menu.h"

#include "core/Config.h"
#include "core/Energy.h"
#include "core/Input.h"
#include "core/Log.h"
#include "core/PowerManager.h"

#include "plugin.h"
#include "CAudioEngine.h"
#include "CFont.h"
#include "CPad.h"
#include "CPlayerPed.h"
#include "CRect.h"
#include "CSprite2d.h"
#include "extensions/ScriptCommands.h"

#include <cstdio>

using namespace plugin;

namespace {

// rows after the powers
enum SettingRow { ROW_ENERGY, ROW_OVERLAY, ROW_CLOSE, SETTING_ROWS };

// layout, in plugin-sdk's reference units: pixels on a 1080-pixel-tall screen (SCREEN_COORD scales them)
const float kLeft = 30.0f;
const float kWidth = 260.0f;
const float kTop = 90.0f;
const float kTitleHeight = 28.0f;
const float kRowHeight = 18.0f;
const float kPadding = 8.0f;

const float kRepeatDelay = 0.40f; // hold an arrow this long before it starts repeating
const float kRepeatRate = 0.10f;  // then one step every this many seconds

void Sound(int event) {
    AudioEngine.ReportFrontendAudioEvent(event, 0.0f, 1.0f);
}

void SetupText(float scaleX, float scaleY, CRGBA color, eFontAlignment align) {
    CFont::SetOrientation(align);
    CFont::SetWrapx(SCREEN_COORD(640.0f));
    CFont::SetRightJustifyWrap(0.0f);
    CFont::SetProportional(true);
    CFont::SetBackground(false, false);
    CFont::SetScale(SCREEN_MULTIPLIER(scaleX), SCREEN_MULTIPLIER(scaleY));
    CFont::SetColor(color);
    CFont::SetDropShadowPosition(1);
    CFont::SetDropColor(CRGBA(0, 0, 0, 255));
}

void Rect(float x1, float y1, float x2, float y2, CRGBA color) {
    CSprite2d::DrawRect(CRect(SCREEN_COORD(x1), SCREEN_COORD(y1), SCREEN_COORD(x2), SCREEN_COORD(y2)), color);
}

} // namespace

void Menu::LoadConfig(const Config& config) {
    // Not F1: in this game F1 makes it stop reading the keyboard (mouse still works) until restart.
    m_key = config.GetKey("Menu", "Key", VK_F10);
    if (m_key == VK_F1) {
        Log::Info("Menu: F1 breaks keyboard input in GTA SA, using F10 instead");
        m_key = VK_F10;
    }
}

int Menu::ItemCount() const {
    return (int)m_powers.Count() + SETTING_ROWS;
}

void Menu::Open() {
    CPlayerPed* player = FindPlayerPed();
    if (!player || player->m_fHealth <= 0.0f)
        return;
    // controls already off = cutscene / mission freeze: don't interfere
    if (!Command<Commands::IS_PLAYER_CONTROL_ON>(0)) {
        Sound(AE_FRONTEND_ERROR);
        return;
    }
    m_open = true;
    // No walking/driving with the arrows, no Enter = enter car. Same game command as
    // CLEO's set_player_control (opcode 01B4).
    Command<Commands::SET_PLAYER_CONTROL>(0, false);
    m_powers.SetHotkeysEnabled(false);
    Sound(AE_FRONTEND_SELECT);
}

void Menu::Close() {
    m_open = false;
    Command<Commands::SET_PLAYER_CONTROL>(0, true);
    m_powers.SetHotkeysEnabled(true);
    Sound(AE_FRONTEND_BACK);
}

bool Menu::Repeat(int vk, float dt, float& heldTime) {
    if (Input::JustPressed(vk)) {
        heldTime = 0.0f;
        return true;
    }
    if (!Input::IsDown(vk)) {
        heldTime = 0.0f;
        return false;
    }
    float before = heldTime;
    heldTime += dt;
    if (heldTime < kRepeatDelay)
        return false;
    // fire once each time we cross a multiple of kRepeatRate past the delay
    return (int)((before - kRepeatDelay) / kRepeatRate) != (int)((heldTime - kRepeatDelay) / kRepeatRate);
}

void Menu::Activate() {
    CPlayerPed* player = FindPlayerPed();
    int index = m_selected;
    if (index < (int)m_powers.Count()) {
        Power& power = m_powers.At(index);
        switch (power.GetMode()) {
        case Power::Mode::Toggle:
            m_powers.SetLevel(power, player, power.IsActive() ? 0 : 1);
            Sound(AE_FRONTEND_SELECT);
            break;
        case Power::Mode::OneShot:
            m_powers.Fire(power, player);
            Sound(AE_FRONTEND_SELECT);
            break;
        case Power::Mode::Hold:
            Sound(AE_FRONTEND_ERROR); // needs its key held; can't be driven from the menu
            break;
        }
        return;
    }
    switch (index - (int)m_powers.Count()) {
    case ROW_ENERGY:
        Energy::SetEnabled(!Energy::IsEnabled());
        Sound(AE_FRONTEND_SELECT);
        break;
    case ROW_OVERLAY:
        m_powers.SetOverlayEnabled(!m_powers.IsOverlayEnabled());
        Sound(AE_FRONTEND_SELECT);
        break;
    case ROW_CLOSE:
        Close();
        break;
    }
}

void Menu::ChangeValue(int delta) {
    CPlayerPed* player = FindPlayerPed();
    int index = m_selected;
    if (index < (int)m_powers.Count()) {
        Power& power = m_powers.At(index);
        if (power.GetMode() != Power::Mode::Toggle) {
            Sound(AE_FRONTEND_ERROR);
            return;
        }
        int level = power.Level() + delta;
        if (level < 0 || level > power.MaxLevel()) {
            Sound(AE_FRONTEND_ERROR);
            return;
        }
        m_powers.SetLevel(power, player, level);
        Sound(AE_FRONTEND_HIGHLIGHT);
        return;
    }
    if (index - (int)m_powers.Count() == ROW_ENERGY) {
        Energy::SetEnabled(delta > 0);
        Sound(AE_FRONTEND_HIGHLIGHT);
    }
    if (index - (int)m_powers.Count() == ROW_OVERLAY) {
        m_powers.SetOverlayEnabled(delta > 0);
        Sound(AE_FRONTEND_HIGHLIGHT);
    }
}

void Menu::Update(float dt) {
    if (Input::JustPressed(m_key)) {
        if (m_open)
            Close();
        else
            Open();
        return;
    }
    if (!m_open)
        return;

    CPlayerPed* player = FindPlayerPed();
    if (!player || player->m_fHealth <= 0.0f) {
        Close(); // died with the menu open
        return;
    }
    int count = ItemCount();
    if (Repeat(VK_UP, dt, m_heldUp)) {
        m_selected = (m_selected + count - 1) % count;
        Sound(AE_FRONTEND_HIGHLIGHT);
    }
    if (Repeat(VK_DOWN, dt, m_heldDown)) {
        m_selected = (m_selected + 1) % count;
        Sound(AE_FRONTEND_HIGHLIGHT);
    }
    if (Repeat(VK_LEFT, dt, m_heldLeft))
        ChangeValue(-1);
    if (Repeat(VK_RIGHT, dt, m_heldRight))
        ChangeValue(+1);
    if (Input::JustPressed(VK_RETURN))
        Activate();
    if (m_open && Input::JustPressed(VK_BACK))
        Close();
}

void Menu::Draw() {
    if (!m_open)
        return;

    int count = ItemCount();
    int powerCount = (int)m_powers.Count();
    // powers, a gap row, settings
    float rowsHeight = (count + 1) * kRowHeight;
    float bottom = kTop + kTitleHeight + rowsHeight + kPadding + 14.0f;

    Rect(kLeft, kTop, kLeft + kWidth, bottom, CRGBA(0, 0, 0, 190));
    Rect(kLeft, kTop, kLeft + kWidth, kTop + kTitleHeight, CRGBA(160, 30, 20, 230));

    CFont::SetFontStyle(FONT_PRICEDOWN);
    SetupText(0.9f, 1.6f, CRGBA(255, 255, 255, 255), ALIGN_LEFT);
    CFont::PrintString(SCREEN_COORD(kLeft + kPadding), SCREEN_COORD(kTop + 2.0f), "Armageddon+");

    char value[32];
    float y = kTop + kTitleHeight + kPadding / 2.0f;
    for (int row = 0; row < count; ++row) {
        if (row == powerCount)
            y += kRowHeight; // gap between powers and settings

        bool selected = row == m_selected;
        if (selected)
            Rect(kLeft + 4.0f, y - 1.0f, kLeft + kWidth - 4.0f, y + kRowHeight - 2.0f, CRGBA(255, 255, 255, 40));

        const char* label;
        CRGBA valueColor(200, 200, 200, 255);
        if (row < powerCount) {
            Power& power = m_powers.At(row);
            label = power.Name();
            switch (power.GetMode()) {
            case Power::Mode::Toggle:
                if (!power.IsActive())
                    std::snprintf(value, sizeof(value), "Off");
                else if (power.MaxLevel() > 1)
                    std::snprintf(value, sizeof(value), "Level %d/%d", power.Level(), power.MaxLevel());
                else
                    std::snprintf(value, sizeof(value), "On");
                if (power.IsActive())
                    valueColor = CRGBA(120, 255, 120, 255);
                break;
            case Power::Mode::OneShot:
                std::snprintf(value, sizeof(value), "[Enter]");
                break;
            case Power::Mode::Hold:
                std::snprintf(value, sizeof(value), "hold %s", Config::KeyName(power.Key()));
                break;
            }
            if (selected && power.GetMode() == Power::Mode::Toggle) {
                char arrows[40];
                std::snprintf(arrows, sizeof(arrows), "< %s >", value);
                std::snprintf(value, sizeof(value), "%s", arrows);
            }
        } else if (row - powerCount == ROW_ENERGY) {
            label = "Energy system";
            std::snprintf(value, sizeof(value), selected ? "< %s >" : "%s", Energy::IsEnabled() ? "On" : "Off (infinite)");
        } else if (row - powerCount == ROW_OVERLAY) {
            label = "HUD power list";
            std::snprintf(value, sizeof(value), selected ? "< %s >" : "%s", m_powers.IsOverlayEnabled() ? "On" : "Off");
        } else {
            label = "Close";
            value[0] = '\0';
        }

        CRGBA labelColor = selected ? CRGBA(255, 200, 60, 255) : CRGBA(230, 230, 230, 255);
        CFont::SetFontStyle(FONT_SUBTITLES);
        SetupText(0.38f, 0.9f, labelColor, ALIGN_LEFT);
        CFont::PrintString(SCREEN_COORD(kLeft + kPadding), SCREEN_COORD(y), label);

        SetupText(0.38f, 0.9f, valueColor, ALIGN_RIGHT);
        CFont::PrintString(SCREEN_COORD(kLeft + kWidth - kPadding), SCREEN_COORD(y), value);

        y += kRowHeight;
    }

    CFont::SetFontStyle(FONT_SUBTITLES);
    SetupText(0.28f, 0.7f, CRGBA(170, 170, 170, 255), ALIGN_LEFT);
    char hint[96];
    std::snprintf(hint, sizeof(hint), "Arrows: select/level  Enter: toggle  %s/Backspace: close", Config::KeyName(m_key));
    CFont::PrintString(SCREEN_COORD(kLeft + kPadding), SCREEN_COORD(y + 2.0f), hint);
}
