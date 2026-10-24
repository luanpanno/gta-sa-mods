#pragma once

#include <functional>
#include <string>
#include <vector>

class Config;
class CPlayerPed;
class PowerManager;
class Power;

// The F10 power menu, driven by the keyboard.
//
//   Menu key (F10)   open / close
//   Up / Down        select (hold to scroll)
//   Left / Right     lower / raise the level of the selected power (Off, 1, 2, 3...)
//   Enter            toggle the selected power (on at its last level), or fire a one-shot power
//   Insert or B      bind a shortcut key to the selected power ("press a key...")
//   Delete           remove the selected power's shortcut
//   C                customize the selected power (speeds, heights, radius... per level, saved)
//   Backspace        close (or cancel binding)
//
// While it's open CJ's controls are disabled (the arrows/Enter would move him or enter cars)
// and the shortcuts are paused; active powers keep running. Shortcuts and settings are saved
// to ArmageddonPlus.user.ini (see UserSettings).
class Menu {
public:
    explicit Menu(PowerManager& powers) : m_powers(powers) {}

    void LoadConfig(const Config& config);
    void Update(float dt); // call from Events::gameProcessEvent, after Input::Update()
    void Draw();           // call from Events::drawHudEvent
    // While open: pin CJ where he stood (controls off alone doesn't stop momentum, the run
    // animation finishing, or powers pushing him). Call every frame, after the powers.
    void HoldPlayer(CPlayerPed* player);

    bool IsOpen() const { return m_open; }
    int Key() const { return m_key; }

private:
    void Open();
    void Close();
    int ItemCount() const;  // powers + settings rows
    void Activate();        // Enter on the selected row
    void ChangeValue(int delta); // Left/Right on the selected row
    void UpdateBinding();   // waiting for the key to bind
    bool Repeat(int vk, float dt, float& heldTime); // pressed now, or held long enough to auto-repeat
    void Notice(const std::string& text); // short message in the footer

    void DrawPowerRow(Power& power, float x1, float x2, float y, bool selected);
    void DrawSettingRow(int setting, float x1, float x2, float y, bool selected);
    void DrawFooter(float x1, float x2, float y);

    // cheats page (opened from the "Cheats" row)
    void UpdateCheats(float dt);
    void DrawCheatsPage();
    void DrawCheatRow(int index, float x1, float x2, float y, bool selected);
    void DrawCheatsFooter(float x1, float x2, float y);

    // shared by both pages: header, scrolling list with section labels, footer
    struct ListItem {
        int row;             // -1 = section label
        const char* section; // label text (section items only)
        bool gapBefore;      // small gap + separator above this row
    };
    void DrawList(const char* subtitle, const std::vector<ListItem>& items, int selectedRow, float& scroll,
                  const std::function<void(int row, float x1, float x2, float y, bool selected)>& drawRow,
                  const std::function<void(float x1, float x2, float y)>& drawFooter);

    // customize page (C on a power row): sliders for the power's settings (see Power::AddSetting)
    enum class CustomRow { Level, Setting, Reset, Back };
    void OpenCustomize(int powerIndex);
    int CustomRowCount() const;
    CustomRow CustomRowKind(int row, int* settingIndex) const;
    void UpdateCustomize(float dt);
    void StepSetting(int setting, int delta, bool toEnd); // Left/Right, Home/End
    void UpdateTyping(int setting);                      // digits typed on a setting row
    void ApplyTyped(int setting);
    void DrawCustomizePage();
    void DrawCustomRow(int row, float x1, float x2, float y, bool selected);
    void DrawCustomFooter(float x1, float x2, float y);
    std::string FormatSetting(const Power& power, int setting, float shown) const;

    enum class Page { Main, Cheats, Customize };

    PowerManager& m_powers;
    Page m_page = Page::Main;
    int m_customPower = 0;       // index in m_powers
    int m_customLevel = 1;       // which level's values the page edits
    int m_customSelected = 0;
    float m_customScroll = 0.0f;
    bool m_holding = false;      // HoldPlayer has a position to keep him at
    float m_holdX = 0.0f, m_holdY = 0.0f;
    bool m_typing = false;       // typing a value on the selected setting
    std::string m_typed;
    int m_cheatSelected = 0;
    float m_cheatScroll = 0.0f;
    int m_key = 0;
    bool m_open = false;
    int m_selected = 0;
    bool m_binding = false;      // waiting for a key press to bind
    std::string m_notice;        // feedback ("Flight: X")
    float m_noticeTime = 0.0f;   // seconds left to show it
    float m_blink = 0.0f;        // animation clock
    float m_scroll = 0.0f;       // list scroll offset (reference units) when it doesn't fit on screen
    float m_heldUp = 0.0f, m_heldDown = 0.0f, m_heldLeft = 0.0f, m_heldRight = 0.0f;
};
