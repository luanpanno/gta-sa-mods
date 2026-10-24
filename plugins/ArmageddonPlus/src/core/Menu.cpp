#include "core/Menu.h"

#include "core/Config.h"
#include "core/Energy.h"
#include "core/Fog.h"
#include "core/Input.h"
#include "core/Log.h"
#include "core/PowerManager.h"
#include "core/Ui.h"
#include "core/UserSettings.h"
#include "cheats/Cheats.h"

#include "plugin.h"
#include "CAudioEngine.h"
#include "CPlayerPed.h"
#include "extensions/ScriptCommands.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <vector>

using namespace plugin;

namespace {

// rows after the powers
enum SettingRow { ROW_CHEATS, ROW_ENERGY, ROW_OVERLAY, ROW_FOG, ROW_CLOSE, SETTING_ROWS };

// layout, in Ui reference units (pixels on a 1080-pixel-tall screen, times [Menu] Scale)
const float kLeft = 48.0f;
const float kWidth = 500.0f;
const float kTop = 60.0f;
const float kHeaderHeight = 66.0f;
const float kRowHeight = 34.0f;
const float kSectionHeight = 30.0f;
const float kPad = 16.0f;
const float kTextSize = 26.0f;     // row text
const float kSmallSize = 20.0f;    // badges, hints
const float kFooterHeight = 140.0f;

const float kRepeatDelay = 0.40f; // hold an arrow this long before it starts repeating
const float kRepeatRate = 0.10f;  // then one step every this many seconds

void Sound(int event) {
    AudioEngine.ReportFrontendAudioEvent(event, 0.0f, 1.0f);
}

bool IsMouseButton(int vk) {
    return vk == VK_LBUTTON || vk == VK_RBUTTON || vk == VK_MBUTTON || vk == VK_XBUTTON1 || vk == VK_XBUTTON2;
}

bool IsSideSpecificModifier(int vk) {
    return vk >= VK_LSHIFT && vk <= VK_RMENU; // we bind the generic SHIFT/CTRL/ALT instead
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

void Menu::Notice(const std::string& text) {
    m_notice = text;
    m_noticeTime = 2.5f;
}

void Menu::Open() {
    CPlayerPed* player = FindPlayerPed();
    if (!player || player->m_fHealth <= 0.0f) {
        Log::Info("Menu: not opened (no player / dead)");
        return;
    }
    // controls already off = cutscene / mission freeze: don't interfere
    if (!Command<Commands::IS_PLAYER_CONTROL_ON>(0)) {
        Log::Info("Menu: not opened, the game has CJ's controls off (cutscene / mission / another script)");
        Notice("The game has CJ's controls off right now");
        Sound(AE_FRONTEND_ERROR);
        return;
    }
    m_open = true;
    m_binding = false;
    m_typing = false;
    m_page = Page::Main;
    // No walking/driving with the arrows, no Enter = enter car. Same game command as
    // CLEO's set_player_control (opcode 01B4).
    Command<Commands::SET_PLAYER_CONTROL>(0, false);
    m_powers.SetHotkeysEnabled(false);
    Sound(AE_FRONTEND_SELECT);
}

void Menu::Close() {
    m_open = false;
    m_binding = false;
    Command<Commands::SET_PLAYER_CONTROL>(0, true);
    m_powers.SetHotkeysEnabled(true);
    Sound(AE_FRONTEND_BACK);
}

void Menu::HoldPlayer(CPlayerPed* player) {
    // only on foot, standing on something: in the air he finishes his jump/fall, flying hovers by itself
    bool hold = m_open && player && !player->bInVehicle && player->bIsStanding && player->bApplyGravity;
    if (!hold) {
        m_holding = false; // anchor again once he's standing
        return;
    }
    CVector pos = player->GetPosition();
    if (!m_holding) {
        m_holding = true;
        m_holdX = pos.x;
        m_holdY = pos.y;
    }
    player->SetPosn(CVector(m_holdX, m_holdY, pos.z)); // height left to the game (slopes, platforms)
    player->m_vecMoveSpeed.x = 0.0f;
    player->m_vecMoveSpeed.y = 0.0f;
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
            m_powers.SetLevel(power, player, power.IsActive() ? 0 : power.LastLevel());
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
    case ROW_CHEATS:
        m_page = Page::Cheats;
        Sound(AE_FRONTEND_SELECT);
        break;
    case ROW_ENERGY:
        ChangeValue(Energy::IsEnabled() ? -1 : +1);
        break;
    case ROW_OVERLAY:
        ChangeValue(m_powers.IsOverlayEnabled() ? -1 : +1);
        break;
    case ROW_FOG:
        ChangeValue(Fog::IsEnabled() ? -1 : +1);
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
        if (power.HasPicker()) {
            // Shift = big steps (e.g. 10 vehicles at a time)
            power.PickerStep(Input::IsDown(VK_SHIFT) ? delta * 10 : delta);
            Sound(AE_FRONTEND_HIGHLIGHT);
            return;
        }
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
    switch (index - (int)m_powers.Count()) {
    case ROW_CHEATS:
        if (delta > 0) {
            m_page = Page::Cheats;
            Sound(AE_FRONTEND_SELECT);
        }
        break;
    case ROW_ENERGY:
        Energy::SetEnabled(delta > 0);
        UserSettings::SaveEnergyEnabled(delta > 0);
        Sound(AE_FRONTEND_HIGHLIGHT);
        break;
    case ROW_OVERLAY:
        m_powers.SetOverlayEnabled(delta > 0);
        UserSettings::SaveOverlayEnabled(delta > 0);
        Sound(AE_FRONTEND_HIGHLIGHT);
        break;
    case ROW_FOG:
        Fog::SetEnabled(delta > 0);
        UserSettings::SaveFogEnabled(delta > 0);
        Sound(AE_FRONTEND_HIGHLIGHT);
        break;
    }
}

void Menu::UpdateBinding() {
    Power& power = m_powers.At(m_selected);
    for (int vk = 1; vk < 256; ++vk) {
        if (!Input::JustPressed(vk) || IsMouseButton(vk) || IsSideSpecificModifier(vk))
            continue;
        if (vk == VK_BACK || vk == VK_ESCAPE) {
            m_binding = false;
            Notice("Cancelled");
            Sound(AE_FRONTEND_BACK);
            return;
        }
        if (vk == VK_F1) {
            Notice("F1 can't be used: it breaks the game's keyboard");
            Sound(AE_FRONTEND_ERROR);
            return;
        }
        if (vk == m_key || vk == VK_UP || vk == VK_DOWN || vk == VK_LEFT || vk == VK_RIGHT || vk == VK_RETURN ||
            vk == VK_INSERT || vk == VK_DELETE) {
            Notice(std::string(Config::KeyName(vk)) + " is used by the menu");
            Sound(AE_FRONTEND_ERROR);
            return;
        }
        Power* previous = m_powers.SetKey(power, vk);
        UserSettings::SaveKey(power.Id(), vk);
        std::string text = std::string(power.Name()) + ": " + Config::KeyName(vk);
        if (previous) {
            UserSettings::SaveKey(previous->Id(), 0);
            text += std::string("  (removed from ") + previous->Name() + ")";
        }
        Notice(text);
        Sound(AE_FRONTEND_SELECT);
        m_binding = false;
        return;
    }
}

void Menu::Update(float dt) {
    m_blink += dt;
    if (m_noticeTime > 0.0f)
        m_noticeTime -= dt;

    if (m_binding) {
        UpdateBinding(); // every key goes to the binding, including the menu key
        return;
    }

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
    if (m_page == Page::Cheats) {
        UpdateCheats(dt);
        return;
    }
    if (m_page == Page::Customize) {
        UpdateCustomize(dt);
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

    bool onPower = m_selected < (int)m_powers.Count();
    if (onPower && Input::JustPressed('C')) {
        if (m_powers.At(m_selected).SettingCount() > 0) {
            OpenCustomize(m_selected);
        } else {
            Notice(std::string(m_powers.At(m_selected).Name()) + " has nothing to customize");
            Sound(AE_FRONTEND_ERROR);
        }
        return;
    }
    if (onPower && (Input::JustPressed(VK_INSERT) || Input::JustPressed('B'))) {
        m_binding = true; // keys are read from the NEXT frame, so Insert itself isn't bound
        Sound(AE_FRONTEND_SELECT);
        return;
    }
    if (onPower && Input::JustPressed(VK_DELETE)) {
        Power& power = m_powers.At(m_selected);
        if (power.Key()) {
            m_powers.SetKey(power, 0);
            UserSettings::SaveKey(power.Id(), 0);
            Notice(std::string(power.Name()) + ": shortcut removed");
            Sound(AE_FRONTEND_BACK);
        }
    }
    if (m_open && Input::JustPressed(VK_BACK))
        Close();
}

// --- drawing ---------------------------------------------------------------------------------

void Menu::DrawPowerRow(Power& power, float x1, float x2, float y, bool selected) {
    float textY = y + (kRowHeight - kTextSize) * 0.5f - 1.0f;
    bool active = power.IsActive();

    // status dot
    float dot = 8.0f, dotY = y + (kRowHeight - dot) * 0.5f;
    Ui::Rect(x1 + kPad, dotY, x1 + kPad + dot, dotY + dot, active ? Ui::kActive : CRGBA(70, 70, 78, 255));

    // name, then the shortcut badge right after it
    float nameX = x1 + kPad + dot + 12.0f;
    CRGBA nameColor = selected ? Ui::kGold : active ? Ui::kText : CRGBA(195, 195, 200, 255);
    Ui::Text(nameX, textY, power.Name(), kTextSize, nameColor);
    float badgeX = nameX + Ui::TextWidth(power.Name(), kTextSize) + 10.0f;
    if (m_binding && selected) {
        if (std::fmod(m_blink, 0.8f) < 0.5f)
            Ui::KeyBadge(badgeX, y + (kRowHeight - kSmallSize) * 0.5f, "?", kSmallSize);
    } else if (power.Key()) {
        Ui::KeyBadge(badgeX, y + (kRowHeight - kSmallSize) * 0.5f, Config::KeyName(power.Key()), kSmallSize);
    }

    // value on the right: level pips + level name / state
    float right = x2 - kPad;
    bool adjustable = power.GetMode() == Power::Mode::Toggle || power.HasPicker(); // Left/Right do something
    if (selected && adjustable) {
        Ui::Text(right, textY, ">", kTextSize, Ui::kGold, Ui::Right);
        right -= 18.0f;
    }
    char value[40];
    CRGBA valueColor = Ui::kTextDim;
    switch (power.GetMode()) {
    case Power::Mode::Toggle:
        if (power.MaxLevel() > 1) {
            float pips = Ui::Pips(right, y + kRowHeight * 0.5f - 5.0f, power.MaxLevel(), power.Level(), 10.0f,
                                  active ? Ui::kActive : Ui::kTextDim);
            right -= pips + 12.0f;
        }
        if (!active)
            std::snprintf(value, sizeof(value), "OFF");
        else if (power.LevelName(power.Level()))
            std::snprintf(value, sizeof(value), "%s", power.LevelName(power.Level()));
        else if (power.MaxLevel() > 1)
            std::snprintf(value, sizeof(value), "LVL %d", power.Level());
        else
            std::snprintf(value, sizeof(value), "ON");
        if (active)
            valueColor = Ui::kActive;
        break;
    case Power::Mode::OneShot:
        std::snprintf(value, sizeof(value), "%s", power.HasPicker() ? power.PickerValue() : "USE");
        valueColor = selected ? Ui::kText : CRGBA(200, 200, 205, 255);
        break;
    case Power::Mode::Hold:
        std::snprintf(value, sizeof(value), "HOLD");
        break;
    }
    Ui::Text(right, textY, value, kTextSize * 0.9f, valueColor, Ui::Right);
    if (selected && adjustable) {
        float w = Ui::TextWidth(value, kTextSize * 0.9f);
        Ui::Text(right - w - 8.0f, textY, "<", kTextSize, Ui::kGold, Ui::Right);
    }
}

void Menu::DrawSettingRow(int setting, float x1, float x2, float y, bool selected) {
    float textY = y + (kRowHeight - kTextSize) * 0.5f - 1.0f;
    const char* label = "";
    const char* value = "";
    bool on = false;
    char cheats[24];
    switch (setting) {
    case ROW_CHEATS:
        label = "Cheats";
        on = Cheats::ActiveCount() > 0;
        std::snprintf(cheats, sizeof(cheats), on ? "%d ON  >>" : ">>", Cheats::ActiveCount());
        value = cheats;
        break;
    case ROW_ENERGY:
        label = "Energy system";
        on = Energy::IsEnabled();
        value = on ? "ON" : "OFF (infinite)";
        break;
    case ROW_OVERLAY:
        label = "HUD power list";
        on = m_powers.IsOverlayEnabled();
        value = on ? "ON" : "OFF";
        break;
    case ROW_FOG:
        label = "Fog";
        on = Fog::IsEnabled();
        value = on ? "ON" : "OFF (clear view)";
        break;
    case ROW_CLOSE:
        label = "Close menu";
        break;
    }
    Ui::Text(x1 + kPad + 20.0f, textY, label, kTextSize, selected ? Ui::kGold : Ui::kText);
    if (*value) {
        float right = x2 - kPad;
        if (selected) {
            Ui::Text(right, textY, ">", kTextSize, Ui::kGold, Ui::Right);
            right -= 18.0f;
        }
        Ui::Text(right, textY, value, kTextSize * 0.9f, on ? Ui::kActive : Ui::kTextDim, Ui::Right);
        if (selected)
            Ui::Text(right - Ui::TextWidth(value, kTextSize * 0.9f) - 8.0f, textY, "<", kTextSize, Ui::kGold, Ui::Right);
    }
}

void Menu::DrawFooter(float x1, float x2, float y) {
    Ui::Rect(x1 + kPad, y, x2 - kPad, y + 1.5f, CRGBA(255, 255, 255, 40));
    y += 10.0f;

    // line 1: what the selected power does / binding prompt / feedback
    std::string line;
    CRGBA color = Ui::kTextDim;
    if (m_binding) {
        line = "Press the key for " + std::string(m_powers.At(m_selected).Name()) + "   (Backspace: cancel)";
        color = Ui::kGold;
    } else if (m_noticeTime > 0.0f) {
        line = m_notice;
        color = Ui::kActive;
    } else if (m_selected < (int)m_powers.Count()) {
        line = m_powers.At(m_selected).Description();
    }
    // wraps inside the panel; room for two lines is reserved so the layout doesn't jump around
    Ui::Text(x1 + kPad, y, line.c_str(), kSmallSize, color, Ui::Left, Ui::FontBody, (x2 - x1) - kPad * 2.0f);
    y += (kSmallSize + 6.0f) * 2.0f + 10.0f;

    // lines 2-3: key hints as badges
    struct Hint { const char* key; const char* label; };
    bool customizable = m_selected < (int)m_powers.Count() && m_powers.At(m_selected).SettingCount() > 0;
    const Hint lines[2][4] = {{{"Up/Down", "select"}, {"Left/Right", "level"}, {"Enter", "on/off"}, {"Bksp", "close"}},
                              {{"Ins", "bind"}, {"Del", "unbind"}, {customizable ? "C" : "", "customize"}, {"", ""}}};
    float size = kSmallSize * 0.85f;
    for (const auto& hints : lines) {
        float x = x1 + kPad;
        for (const Hint& hint : hints) {
            if (!*hint.key)
                continue;
            x += Ui::KeyBadge(x, y, hint.key, size) + 5.0f;
            Ui::Text(x, y, hint.label, size, Ui::kTextDim);
            x += Ui::TextWidth(hint.label, size) + 14.0f;
        }
        y += size + 12.0f;
    }
}

// --- generic scrolling list (used by both pages) ---------------------------------------------

void Menu::DrawList(const char* subtitle, const std::vector<ListItem>& items, int selectedRow, float& scroll,
                    const std::function<void(int row, float x1, float x2, float y, bool selected)>& drawRow,
                    const std::function<void(float x1, float x2, float y)>& drawFooter) {
    // 1) lay out in "content" coordinates
    std::vector<float> tops(items.size());
    float contentY = 0.0f, selectedTop = 0.0f;
    for (size_t i = 0; i < items.size(); ++i) {
        if (items[i].gapBefore)
            contentY += 14.0f;
        tops[i] = contentY;
        if (items[i].row == selectedRow)
            selectedTop = contentY;
        contentY += items[i].row < 0 ? kSectionHeight : kRowHeight;
    }

    // 2) fit the panel on screen; if the list is taller, scroll so the selected row stays visible
    float listHeight = Ui::ScreenHeight() - kTop * 2.0f - kHeaderHeight - kFooterHeight;
    if (listHeight > contentY)
        listHeight = contentY;
    float margin = kRowHeight * 1.5f;
    if (selectedTop - scroll < margin)
        scroll = selectedTop - margin;
    if (selectedTop + kRowHeight - scroll > listHeight - margin)
        scroll = selectedTop + kRowHeight - listHeight + margin;
    if (scroll > contentY - listHeight)
        scroll = contentY - listHeight;
    if (scroll < 0.0f)
        scroll = 0.0f;

    float x1 = kLeft, x2 = kLeft + kWidth, y = kTop;
    float height = kHeaderHeight + listHeight + kFooterHeight;

    // panel: drop shadow, dark gradient body, thin red outline
    Ui::Rect(x1 + 6.0f, y + 6.0f, x2 + 6.0f, y + height + 6.0f, CRGBA(0, 0, 0, 90));
    Ui::GradientV(x1, y, x2, y + height, CRGBA(18, 14, 18, 230), CRGBA(6, 6, 10, 235));
    Ui::Frame(x1, y, x2, y + height, 1.5f, CRGBA(215, 40, 30, 160));

    // header: red gradient + title + subtitle + energy readout
    Ui::GradientH(x1, y, x2, y + kHeaderHeight, Ui::kAccent, Ui::kAccentDark);
    Ui::Text(x1 + kPad, y + 6.0f, "Armageddon+", 46.0f, CRGBA(255, 255, 255, 255), Ui::Left, Ui::FontTitle);
    Ui::Text(x2 - kPad, y + 12.0f, subtitle, 18.0f, CRGBA(255, 220, 210, 230), Ui::Right, Ui::FontHeader);
    if (Energy::IsEnabled()) {
        char energy[32];
        std::snprintf(energy, sizeof(energy), "ENERGY %d", (int)std::ceil(Energy::Current()));
        Ui::Text(x2 - kPad, y + 36.0f, energy, 18.0f, CRGBA(170, 210, 255, 255), Ui::Right, Ui::FontHeader);
    }
    float listTop = y + kHeaderHeight;

    // 3) draw the items fully inside the visible window
    for (size_t i = 0; i < items.size(); ++i) {
        const ListItem& item = items[i];
        float itemHeight = item.row < 0 ? kSectionHeight : kRowHeight;
        float top = tops[i] - scroll;
        if (top < -0.5f || top + itemHeight > listHeight + 0.5f)
            continue;
        float iy = listTop + top;
        if (item.row < 0) {
            Ui::Text(x1 + kPad, iy + 6.0f, item.section, 18.0f, CRGBA(255, 110, 95, 255), Ui::Left, Ui::FontHeader);
            float lineX = x1 + kPad + Ui::TextWidth(item.section, 18.0f, Ui::FontHeader) + 10.0f;
            Ui::GradientH(lineX, iy + 15.0f, x2 - kPad, iy + 16.5f, CRGBA(215, 40, 30, 180), CRGBA(215, 40, 30, 0));
            continue;
        }
        if (item.gapBefore)
            Ui::Rect(x1 + kPad, iy - 8.0f, x2 - kPad, iy - 6.5f, CRGBA(255, 255, 255, 30));

        bool selected = item.row == selectedRow;
        if (selected) {
            // highlight: red glow fading to the right + accent bar on the left
            Ui::GradientH(x1 + 2.0f, iy, x2 - 2.0f, iy + kRowHeight, CRGBA(215, 40, 30, 120), CRGBA(215, 40, 30, 10));
            Ui::Rect(x1 + 2.0f, iy, x1 + 7.0f, iy + kRowHeight, Ui::kAccent);
        }
        drawRow(item.row, x1, x2, iy, selected);
    }

    // "more" hints when the list is scrolled
    if (scroll > 0.5f)
        Ui::Text(x2 - kPad, listTop + 2.0f, "^ more", 16.0f, Ui::kTextDim, Ui::Right);
    if (scroll + listHeight < contentY - 0.5f)
        Ui::Text(x2 - kPad, listTop + listHeight - 18.0f, "v more", 16.0f, Ui::kTextDim, Ui::Right);

    drawFooter(x1, x2, listTop + listHeight + 6.0f);
}

void Menu::Draw() {
    if (!m_open)
        return;
    if (m_page == Page::Cheats) {
        DrawCheatsPage();
        return;
    }
    if (m_page == Page::Customize) {
        DrawCustomizePage();
        return;
    }

    // main page: power rows (with a section label wherever the category changes), then settings
    int powerCount = (int)m_powers.Count();
    std::vector<ListItem> items;
    for (int row = 0; row < ItemCount(); ++row) {
        if (row < powerCount && (row == 0 || std::strcmp(m_powers.At(row).Category(), m_powers.At(row - 1).Category()) != 0))
            items.push_back({-1, m_powers.At(row).Category(), false});
        items.push_back({row, nullptr, row == powerCount}); // gap before the settings
    }
    DrawList("POWER MENU", items, m_selected, m_scroll,
             [this, powerCount](int row, float x1, float x2, float y, bool selected) {
                 if (row < powerCount)
                     DrawPowerRow(m_powers.At(row), x1, x2, y, selected);
                 else
                     DrawSettingRow(row - powerCount, x1, x2, y, selected);
             },
             [this](float x1, float x2, float y) { DrawFooter(x1, x2, y); });
}

// --- cheats page -----------------------------------------------------------------------------

void Menu::UpdateCheats(float dt) {
    int count = Cheats::Count();
    if (Repeat(VK_UP, dt, m_heldUp)) {
        m_cheatSelected = (m_cheatSelected + count - 1) % count;
        Sound(AE_FRONTEND_HIGHLIGHT);
    }
    if (Repeat(VK_DOWN, dt, m_heldDown)) {
        m_cheatSelected = (m_cheatSelected + 1) % count;
        Sound(AE_FRONTEND_HIGHLIGHT);
    }
    const Cheats::Info& cheat = Cheats::Get(m_cheatSelected);
    bool apply = Input::JustPressed(VK_RETURN);
    // Left/Right on a toggle cheat: switch it off / on (Enter flips it either way)
    if (!cheat.oneTime && Input::JustPressed(VK_LEFT) && Cheats::IsActive(m_cheatSelected))
        apply = true;
    if (!cheat.oneTime && Input::JustPressed(VK_RIGHT) && !Cheats::IsActive(m_cheatSelected))
        apply = true;
    if (apply) {
        Cheats::Apply(m_cheatSelected);
        if (cheat.oneTime)
            Notice(std::string(cheat.name) + ": done");
        else
            Notice(std::string(cheat.name) + (Cheats::IsActive(m_cheatSelected) ? ": ON" : ": OFF"));
        Sound(AE_FRONTEND_SELECT);
    }
    if (Input::JustPressed(VK_BACK)) {
        m_page = Page::Main; // back to the power menu (F10 closes everything)
        Sound(AE_FRONTEND_BACK);
    }
}

void Menu::DrawCheatRow(int index, float x1, float x2, float y, bool selected) {
    const Cheats::Info& cheat = Cheats::Get(index);
    float textY = y + (kRowHeight - kTextSize) * 0.5f - 1.0f;
    bool active = !cheat.oneTime && Cheats::IsActive(index);

    float dot = 8.0f, dotY = y + (kRowHeight - dot) * 0.5f;
    if (!cheat.oneTime)
        Ui::Rect(x1 + kPad, dotY, x1 + kPad + dot, dotY + dot, active ? Ui::kActive : CRGBA(70, 70, 78, 255));
    CRGBA nameColor = selected ? Ui::kGold : active ? Ui::kText : CRGBA(195, 195, 200, 255);
    Ui::Text(x1 + kPad + dot + 12.0f, textY, cheat.name, kTextSize * 0.92f, nameColor);

    const char* value = cheat.oneTime ? "USE" : active ? "ON" : "OFF";
    CRGBA valueColor = cheat.oneTime ? (selected ? Ui::kText : Ui::kTextDim) : active ? Ui::kActive : Ui::kTextDim;
    Ui::Text(x2 - kPad, textY, value, kTextSize * 0.9f, valueColor, Ui::Right);
}

void Menu::DrawCheatsFooter(float x1, float x2, float y) {
    Ui::Rect(x1 + kPad, y, x2 - kPad, y + 1.5f, CRGBA(255, 255, 255, 40));
    y += 10.0f;

    const Cheats::Info& cheat = Cheats::Get(m_cheatSelected);
    std::string line;
    CRGBA color = Ui::kTextDim;
    if (m_noticeTime > 0.0f) {
        line = m_notice;
        color = Ui::kActive;
    } else if (cheat.oneTime) {
        line = "One-time cheat: Enter uses it, exactly like typing its code.";
    } else {
        line = "Toggle cheat: stays on until you switch it off. Active cheats show in the HUD.";
    }
    Ui::Text(x1 + kPad, y, line.c_str(), kSmallSize, color, Ui::Left, Ui::FontBody, (x2 - x1) - kPad * 2.0f);
    y += (kSmallSize + 6.0f) * 2.0f + 10.0f;

    struct Hint { const char* key; const char* label; };
    const Hint lines[2][3] = {{{"Up/Down", "select"}, {"Enter", "use / on-off"}, {"Left/Right", "off / on"}},
                              {{"Bksp", "back to powers"}, {"F10", "close"}, {"", ""}}};
    float size = kSmallSize * 0.85f;
    for (const auto& hints : lines) {
        float x = x1 + kPad;
        for (const Hint& hint : hints) {
            if (!*hint.key)
                continue;
            x += Ui::KeyBadge(x, y, hint.key, size) + 5.0f;
            Ui::Text(x, y, hint.label, size, Ui::kTextDim);
            x += Ui::TextWidth(hint.label, size) + 14.0f;
        }
        y += size + 12.0f;
    }
}

void Menu::DrawCheatsPage() {
    std::vector<ListItem> items;
    for (int i = 0; i < Cheats::Count(); ++i) {
        if (i == 0 || std::strcmp(Cheats::Get(i).group, Cheats::Get(i - 1).group) != 0)
            items.push_back({-1, Cheats::Get(i).group, false});
        items.push_back({i, nullptr, false});
    }
    char subtitle[32];
    std::snprintf(subtitle, sizeof(subtitle), "CHEATS  %d ON", Cheats::ActiveCount());
    DrawList(subtitle, items, m_cheatSelected, m_cheatScroll,
             [this](int row, float x1, float x2, float y, bool selected) { DrawCheatRow(row, x1, x2, y, selected); },
             [this](float x1, float x2, float y) { DrawCheatsFooter(x1, x2, y); });
}
// --- customize page --------------------------------------------------------------------------

void Menu::OpenCustomize(int powerIndex) {
    Power& power = m_powers.At(powerIndex);
    m_customPower = powerIndex;
    // edit the level it's on (or comes back at): the one you're most likely tuning
    m_customLevel = power.IsActive() && power.Level() > 0 ? power.Level() : power.LastLevel();
    if (m_customLevel < 1 || m_customLevel > power.MaxLevel())
        m_customLevel = 1;
    m_customSelected = 0;
    m_customScroll = 0.0f;
    m_typing = false;
    m_page = Page::Customize;
    Sound(AE_FRONTEND_SELECT);
}

// rows: [level picker], one per setting, Reset, Back
int Menu::CustomRowCount() const {
    const Power& power = m_powers.At(m_customPower);
    bool levelRow = power.HasPerLevelSettings() && power.MaxLevel() > 1;
    return (levelRow ? 1 : 0) + power.SettingCount() + 2;
}

Menu::CustomRow Menu::CustomRowKind(int row, int* settingIndex) const {
    const Power& power = m_powers.At(m_customPower);
    if (power.HasPerLevelSettings() && power.MaxLevel() > 1) {
        if (row == 0)
            return CustomRow::Level;
        --row;
    }
    if (row < power.SettingCount()) {
        if (settingIndex)
            *settingIndex = row;
        return CustomRow::Setting;
    }
    return row == power.SettingCount() ? CustomRow::Reset : CustomRow::Back;
}

std::string Menu::FormatSetting(const Power& power, int setting, float shown) const {
    const Power::Setting& info = power.GetSetting(setting);
    int decimals = info.step >= 1.0f ? 0 : info.step >= 0.1f ? 1 : 2;
    bool space = *info.unit && info.unit[0] != 'x' && info.unit[0] != '%'; // "30 m/s", but "4.5x", "35%"
    char text[32];
    std::snprintf(text, sizeof(text), "%.*f%s%s", decimals, shown, space ? " " : "", info.unit);
    return text;
}

void Menu::StepSetting(int setting, int delta, bool toEnd) {
    Power& power = m_powers.At(m_customPower);
    const Power::Setting& info = power.GetSetting(setting);
    float shown = power.SettingShown(setting, m_customLevel);
    float next;
    if (toEnd) {
        next = delta > 0 ? info.max : info.min;
    } else {
        int steps = Input::IsDown(VK_SHIFT) ? delta * 10 : delta; // Shift = big steps
        next = std::round(shown / info.step + steps) * info.step;  // stays on the step grid
    }
    next = std::clamp(next, info.min, info.max);
    if (std::fabs(next - shown) < info.step * 0.01f) {
        Sound(AE_FRONTEND_ERROR); // already at the end
        return;
    }
    power.SetSettingShown(setting, m_customLevel, next);
    UserSettings::SaveSetting(power, setting, m_customLevel);
    Sound(AE_FRONTEND_HIGHLIGHT);
}

void Menu::ApplyTyped(int setting) {
    Power& power = m_powers.At(m_customPower);
    const Power::Setting& info = power.GetSetting(setting);
    std::string text = m_typed;
    m_typing = false;
    if (text.empty() || text == ".")
        return;
    float value = (float)std::atof(text.c_str());
    float clamped = std::clamp(value, info.min, info.max);
    power.SetSettingShown(setting, m_customLevel, clamped);
    UserSettings::SaveSetting(power, setting, m_customLevel);
    if (clamped != value)
        Notice(std::string(clamped > value ? "Min is " : "Max is ") + FormatSetting(power, setting, clamped));
    Sound(AE_FRONTEND_SELECT);
}

void Menu::UpdateTyping(int setting) {
    // digits from the number row or the numpad; '.' or ',' (decimal comma keyboards) for decimals
    for (int vk = 0; vk < 256; ++vk) {
        if (!Input::JustPressed(vk))
            continue;
        char c = 0;
        if (vk >= '0' && vk <= '9')
            c = (char)vk;
        else if (vk >= VK_NUMPAD0 && vk <= VK_NUMPAD9)
            c = (char)('0' + vk - VK_NUMPAD0);
        else if (vk == VK_OEM_PERIOD || vk == VK_DECIMAL || vk == VK_OEM_COMMA)
            c = '.';
        if (!c)
            continue;
        if (!m_typing) {
            m_typing = true;
            m_typed.clear();
        }
        bool secondDot = c == '.' && m_typed.find('.') != std::string::npos;
        if (m_typed.size() < 8 && !secondDot)
            m_typed += c;
        Sound(AE_FRONTEND_HIGHLIGHT);
    }
}

void Menu::UpdateCustomize(float dt) {
    Power& power = m_powers.At(m_customPower);
    int count = CustomRowCount();
    int setting = -1;
    CustomRow kind = CustomRowKind(m_customSelected, &setting);

    if (m_typing) {
        // typing a value: digits, Enter (apply), Backspace (delete / cancel); arrows cancel
        UpdateTyping(setting);
        if (Input::JustPressed(VK_RETURN)) {
            ApplyTyped(setting);
        } else if (Input::JustPressed(VK_BACK)) {
            m_typed.pop_back();
            if (m_typed.empty())
                m_typing = false;
            Sound(AE_FRONTEND_BACK);
        } else if (Input::JustPressed(VK_UP) || Input::JustPressed(VK_DOWN) || Input::JustPressed(VK_LEFT) ||
                   Input::JustPressed(VK_RIGHT)) {
            m_typing = false;
            Sound(AE_FRONTEND_BACK);
        }
        return;
    }

    if (Repeat(VK_UP, dt, m_heldUp)) {
        m_customSelected = (m_customSelected + count - 1) % count;
        Sound(AE_FRONTEND_HIGHLIGHT);
    }
    if (Repeat(VK_DOWN, dt, m_heldDown)) {
        m_customSelected = (m_customSelected + 1) % count;
        Sound(AE_FRONTEND_HIGHLIGHT);
    }
    kind = CustomRowKind(m_customSelected, &setting);

    int delta = 0;
    if (Repeat(VK_LEFT, dt, m_heldLeft))
        delta = -1;
    if (Repeat(VK_RIGHT, dt, m_heldRight))
        delta = +1;
    switch (kind) {
    case CustomRow::Level:
        if (delta) {
            int level = m_customLevel + delta;
            if (level >= 1 && level <= power.MaxLevel()) {
                m_customLevel = level;
                Sound(AE_FRONTEND_HIGHLIGHT);
            } else {
                Sound(AE_FRONTEND_ERROR);
            }
        }
        break;
    case CustomRow::Setting:
        if (delta)
            StepSetting(setting, delta, false);
        if (Input::JustPressed(VK_HOME))
            StepSetting(setting, -1, true);
        if (Input::JustPressed(VK_END))
            StepSetting(setting, +1, true);
        UpdateTyping(setting);
        if (m_typing)
            return;
        break;
    case CustomRow::Reset:
        if (Input::JustPressed(VK_RETURN)) {
            power.ResetSettings();
            UserSettings::ClearSettings(power);
            Notice(std::string(power.Name()) + ": back to the default values");
            Sound(AE_FRONTEND_SELECT);
        }
        break;
    case CustomRow::Back:
        if (Input::JustPressed(VK_RETURN)) {
            m_page = Page::Main;
            Sound(AE_FRONTEND_BACK);
            return;
        }
        break;
    }
    if (Input::JustPressed(VK_BACK)) {
        m_page = Page::Main; // back to the power menu (F10 closes everything)
        Sound(AE_FRONTEND_BACK);
    }
}

void Menu::DrawCustomRow(int row, float x1, float x2, float y, bool selected) {
    const Power& power = m_powers.At(m_customPower);
    float textY = y + (kRowHeight - kTextSize) * 0.5f - 1.0f;
    float labelX = x1 + kPad + 20.0f;
    float right = x2 - kPad;
    int setting = -1;
    CRGBA labelColor = selected ? Ui::kGold : Ui::kText;

    switch (CustomRowKind(row, &setting)) {
    case CustomRow::Level: {
        Ui::Text(labelX, textY, "Editing level", kTextSize, labelColor);
        if (selected) {
            Ui::Text(right, textY, ">", kTextSize, Ui::kGold, Ui::Right);
            right -= 18.0f;
        }
        right -= Ui::Pips(right, y + kRowHeight * 0.5f - 5.0f, power.MaxLevel(), m_customLevel, 10.0f, Ui::kActive) + 12.0f;
        char value[40];
        if (power.LevelName(m_customLevel))
            std::snprintf(value, sizeof(value), "%s", power.LevelName(m_customLevel));
        else
            std::snprintf(value, sizeof(value), "LVL %d", m_customLevel);
        Ui::Text(right, textY, value, kTextSize * 0.9f, Ui::kActive, Ui::Right);
        if (selected)
            Ui::Text(right - Ui::TextWidth(value, kTextSize * 0.9f) - 8.0f, textY, "<", kTextSize, Ui::kGold, Ui::Right);
        break;
    }
    case CustomRow::Setting: {
        const Power::Setting& info = power.GetSetting(setting);
        float shown = power.SettingShown(setting, m_customLevel);
        Ui::Text(labelX, textY, info.label, kTextSize * 0.9f, labelColor);

        // on the right: < value [slider from min to max] >
        std::string value;
        CRGBA valueColor = Ui::kText;
        if (selected && m_typing) {
            value = m_typed + (std::fmod(m_blink, 0.8f) < 0.5f ? "_" : " ");
            valueColor = Ui::kGold;
        } else {
            value = FormatSetting(power, setting, shown);
        }
        if (selected && !m_typing) {
            Ui::Text(right, textY, ">", kTextSize, Ui::kGold, Ui::Right);
            right -= 18.0f;
        }
        const float barWidth = 110.0f;
        float barLeft = right - barWidth;
        float barY = y + kRowHeight * 0.5f - 3.0f;
        float fill = info.max > info.min ? (shown - info.min) / (info.max - info.min) : 0.0f;
        fill = std::clamp(fill, 0.0f, 1.0f);
        Ui::Rect(barLeft, barY, right, barY + 6.0f, CRGBA(255, 255, 255, 35));
        Ui::GradientH(barLeft, barY, barLeft + barWidth * fill, barY + 6.0f, Ui::kAccentDark,
                      fill >= 0.999f ? Ui::kGold : Ui::kAccent); // gold = at the max
        float valueRight = barLeft - 10.0f;
        Ui::Text(valueRight, textY, value.c_str(), kTextSize * 0.85f, valueColor, Ui::Right);
        if (selected && !m_typing)
            Ui::Text(valueRight - Ui::TextWidth(value.c_str(), kTextSize * 0.85f) - 8.0f, textY, "<", kTextSize,
                     Ui::kGold, Ui::Right);
        break;
    }
    case CustomRow::Reset:
        Ui::Text(labelX, textY, "Reset to defaults", kTextSize, labelColor);
        break;
    case CustomRow::Back:
        Ui::Text(labelX, textY, "Back", kTextSize, labelColor);
        break;
    }
}

void Menu::DrawCustomFooter(float x1, float x2, float y) {
    const Power& power = m_powers.At(m_customPower);
    Ui::Rect(x1 + kPad, y, x2 - kPad, y + 1.5f, CRGBA(255, 255, 255, 40));
    y += 10.0f;

    int setting = -1;
    CustomRow kind = CustomRowKind(m_customSelected, &setting);
    std::string line;
    CRGBA color = Ui::kTextDim;
    if (m_noticeTime > 0.0f) {
        line = m_notice;
        color = Ui::kActive;
    } else if (kind == CustomRow::Setting) {
        const Power::Setting& info = power.GetSetting(setting);
        line = "From " + FormatSetting(power, setting, info.min) + " to " + FormatSetting(power, setting, info.max) +
               " (max). ";
        if (m_typing) {
            line += "Enter: apply, Bksp: delete, arrows: cancel.";
            color = Ui::kGold;
        } else if (info.perLevel && power.MaxLevel() > 1) {
            line += "Level " + std::to_string(m_customLevel) + " only. Saved; applies right away.";
        } else {
            line += "Every level. Saved; applies right away.";
        }
    } else if (kind == CustomRow::Level) {
        line = "Each level has its own values: pick the one to edit.";
    } else if (kind == CustomRow::Reset) {
        line = "Enter: put every value back to the ini defaults.";
    }
    Ui::Text(x1 + kPad, y, line.c_str(), kSmallSize, color, Ui::Left, Ui::FontBody, (x2 - x1) - kPad * 2.0f);
    y += (kSmallSize + 6.0f) * 2.0f + 10.0f;

    struct Hint { const char* key; const char* label; };
    const Hint lines[2][3] = {{{"Up/Down", "select"}, {"Left/Right", "change"}, {"Shift", "faster"}},
                              {{"Home/End", "min/max"}, {"0-9", "type a value"}, {"Bksp", "back"}}};
    float size = kSmallSize * 0.85f;
    for (const auto& hints : lines) {
        float x = x1 + kPad;
        for (const Hint& hint : hints) {
            x += Ui::KeyBadge(x, y, hint.key, size) + 5.0f;
            Ui::Text(x, y, hint.label, size, Ui::kTextDim);
            x += Ui::TextWidth(hint.label, size) + 14.0f;
        }
        y += size + 12.0f;
    }
}

void Menu::DrawCustomizePage() {
    const Power& power = m_powers.At(m_customPower);
    std::vector<ListItem> items;
    items.push_back({-1, power.Name(), false});
    bool levelRow = CustomRowKind(0, nullptr) == CustomRow::Level;
    for (int row = 0; row < CustomRowCount(); ++row) {
        bool gap = CustomRowKind(row, nullptr) == CustomRow::Reset || (levelRow && row == 1);
        items.push_back({row, nullptr, gap});
    }
    DrawList("CUSTOMIZE", items, m_customSelected, m_customScroll,
             [this](int row, float x1, float x2, float y, bool selected) { DrawCustomRow(row, x1, x2, y, selected); },
             [this](float x1, float x2, float y) { DrawCustomFooter(x1, x2, y); });
}
