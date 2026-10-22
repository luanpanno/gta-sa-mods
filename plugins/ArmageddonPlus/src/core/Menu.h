#pragma once

class Config;
class PowerManager;

// Phase 4.1: on-screen power menu, driven by the keyboard.
//
//   Menu key (F10)  open / close
//   Up / Down       select (hold to scroll)
//   Left / Right    lower / raise the level (Off, 1, 2, 3) of the selected power
//   Enter           toggle the selected power on/off, or fire a one-shot power
//   Backspace       close
//
// While it's open CJ's controls are disabled (the arrows/Enter would move him or enter cars)
// and the power hotkeys are paused; active powers keep running.
class Menu {
public:
    explicit Menu(PowerManager& powers) : m_powers(powers) {}

    void LoadConfig(const Config& config);
    void Update(float dt); // call from Events::gameProcessEvent, after Input::Update()
    void Draw();           // call from Events::drawHudEvent

    bool IsOpen() const { return m_open; }

private:
    void Open();
    void Close();
    int ItemCount() const;  // powers + settings rows
    void Activate();        // Enter on the selected row
    void ChangeValue(int delta); // Left/Right on the selected row
    bool Repeat(int vk, float dt, float& heldTime); // pressed now, or held long enough to auto-repeat

    PowerManager& m_powers;
    int m_key = 0;
    bool m_open = false;
    int m_selected = 0;
    float m_heldUp = 0.0f, m_heldDown = 0.0f, m_heldLeft = 0.0f, m_heldRight = 0.0f;
};
