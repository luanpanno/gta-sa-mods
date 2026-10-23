#pragma once

#include "CRGBA.h"

class Config;

// Small drawing toolkit shared by the menu, the power list and the energy bar.
//
// All positions and sizes are in REFERENCE UNITS: pixels on a 1080-pixel-tall screen, times the
// [Menu] Scale setting. Ui converts them to real pixels, so the HUD looks the same at any resolution.
namespace Ui {

// palette
extern const CRGBA kAccent;      // Armageddon red
extern const CRGBA kAccentDark;
extern const CRGBA kPanel;       // dark translucent background
extern const CRGBA kText;
extern const CRGBA kTextDim;
extern const CRGBA kActive;      // green: "on"
extern const CRGBA kGold;        // selected row label

void LoadConfig(const Config& config);

float ScreenWidth();  // in reference units (e.g. 1920 at 1080p, 2560 at 1440p ultrawide...)
float ScreenHeight(); // in reference units (1080 / Scale)

void Rect(float x1, float y1, float x2, float y2, CRGBA color);
void GradientH(float x1, float y1, float x2, float y2, CRGBA left, CRGBA right);
void GradientV(float x1, float y1, float x2, float y2, CRGBA top, CRGBA bottom);
void Frame(float x1, float y1, float x2, float y2, float thickness, CRGBA color); // outline

enum Font { FontBody, FontTitle, FontHeader }; // subtitles / pricedown / menu
enum Align { Left, Right, Center };
// size = text height-ish in reference units (body text ~ 26).
// wrapWidth > 0 (left-aligned text): break into lines so it never goes past x + wrapWidth.
void Text(float x, float y, const char* text, float size, CRGBA color, Align align = Left, Font font = FontBody,
          float wrapWidth = 0.0f);
float TextWidth(const char* text, float size, Font font = FontBody);

// keyboard-key badge, e.g. [F10]. Returns its width. x is its left edge (or right edge if rightAligned).
float KeyBadge(float x, float y, const char* key, float size, bool rightAligned = false);
// level pips ■■□, drawn leftward from xRight. Returns their width.
float Pips(float xRight, float y, int max, int filled, float height, CRGBA on);

} // namespace Ui
