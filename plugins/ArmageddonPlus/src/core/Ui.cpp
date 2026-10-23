#include "core/Ui.h"

#include "core/Config.h"

#include "plugin.h"
#include "CFont.h"
#include "CRect.h"
#include "CSprite2d.h"

using namespace plugin;

namespace {

float g_scale = 1.0f; // [Menu] Scale

float Px(float units) { // reference units -> real pixels
    return SCREEN_MULTIPLIER(units * g_scale);
}

void SetupFont(float size, CRGBA color, Ui::Align align, Ui::Font font) {
    switch (font) {
    case Ui::FontTitle:  CFont::SetFontStyle(FONT_PRICEDOWN); break;
    case Ui::FontHeader: CFont::SetFontStyle(FONT_MENU); break;
    default:             CFont::SetFontStyle(FONT_SUBTITLES); break;
    }
    CFont::SetOrientation(align == Ui::Right ? ALIGN_RIGHT : align == Ui::Center ? ALIGN_CENTER : ALIGN_LEFT);
    CFont::SetWrapx(SCREEN_WIDTH * 2.0f);
    CFont::SetRightJustifyWrap(0.0f);
    CFont::SetCentreSize(SCREEN_WIDTH * 2.0f);
    CFont::SetProportional(true);
    CFont::SetBackground(false, false);
    CFont::SetJustify(false);
    // CFont scale: y ~ 1/22 of the glyph cell height, x ~ 0.45 of y looks right for these fonts
    float sy = size / 22.0f;
    float sx = sy * (font == Ui::FontTitle ? 0.55f : 0.45f);
    CFont::SetScale(Px(sx), Px(sy));
    CFont::SetColor(color);
    CFont::SetDropShadowPosition(font == Ui::FontTitle ? 2 : 1);
    CFont::SetDropColor(CRGBA(0, 0, 0, color.a));
}

} // namespace

namespace Ui {

const CRGBA kAccent(215, 40, 30, 255);
const CRGBA kAccentDark(90, 10, 8, 255);
const CRGBA kPanel(8, 8, 12, 215);
const CRGBA kText(235, 235, 235, 255);
const CRGBA kTextDim(150, 150, 155, 255);
const CRGBA kActive(110, 240, 120, 255);
const CRGBA kGold(255, 200, 70, 255);

void LoadConfig(const Config& config) {
    g_scale = config.GetFloat("Menu", "Scale", 1.0f);
    if (g_scale < 0.5f)
        g_scale = 0.5f;
    if (g_scale > 2.5f)
        g_scale = 2.5f;
}

float ScreenWidth() {
    return SCREEN_WIDTH / Px(1.0f);
}

float ScreenHeight() {
    return SCREEN_HEIGHT / Px(1.0f);
}

void Rect(float x1, float y1, float x2, float y2, CRGBA color) {
    CSprite2d::DrawRect(CRect(Px(x1), Px(y1), Px(x2), Px(y2)), color);
}

void GradientH(float x1, float y1, float x2, float y2, CRGBA left, CRGBA right) {
    // vertices: top-left, top-right, bottom-left, bottom-right (triangle strip order)
    CSprite2d::DrawAnyRect(Px(x1), Px(y1), Px(x2), Px(y1), Px(x1), Px(y2), Px(x2), Px(y2), left, right, left, right);
}

void GradientV(float x1, float y1, float x2, float y2, CRGBA top, CRGBA bottom) {
    CSprite2d::DrawAnyRect(Px(x1), Px(y1), Px(x2), Px(y1), Px(x1), Px(y2), Px(x2), Px(y2), top, top, bottom, bottom);
}

void Frame(float x1, float y1, float x2, float y2, float t, CRGBA color) {
    Rect(x1, y1, x2, y1 + t, color);
    Rect(x1, y2 - t, x2, y2, color);
    Rect(x1, y1, x1 + t, y2, color);
    Rect(x2 - t, y1, x2, y2, color);
}

void Text(float x, float y, const char* text, float size, CRGBA color, Align align, Font font, float wrapWidth) {
    SetupFont(size, color, align, font);
    if (wrapWidth > 0.0f)
        CFont::SetWrapx(Px(x + wrapWidth)); // the game's font engine wraps at this screen x
    CFont::PrintString(Px(x), Px(y), text);
}

float TextWidth(const char* text, float size, Font font) {
    SetupFont(size, CRGBA(255, 255, 255, 255), Left, font);
    return CFont::GetStringWidth(text, true) / Px(1.0f);
}

float KeyBadge(float x, float y, const char* key, float size, bool rightAligned) {
    float pad = size * 0.35f;
    float width = TextWidth(key, size) + pad * 2.0f;
    float left = rightAligned ? x - width : x;
    float top = y - size * 0.05f, bottom = y + size * 1.05f;
    Rect(left, top, left + width, bottom, CRGBA(45, 45, 52, 235));
    Frame(left, top, left + width, bottom, 1.5f, CRGBA(120, 120, 130, 255));
    Text(left + pad, y, key, size, kText);
    return width;
}

float Pips(float xRight, float y, int max, int filled, float height, CRGBA on) {
    float w = height * 1.6f, gap = height * 0.5f;
    float x = xRight;
    for (int i = max; i >= 1; --i) {
        float left = x - w;
        Rect(left, y, x, y + height, i <= filled ? on : CRGBA(70, 70, 78, 230));
        x = left - gap;
    }
    return xRight - (x + gap);
}

} // namespace Ui
