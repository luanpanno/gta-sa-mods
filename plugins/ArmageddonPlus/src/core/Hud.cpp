#include "core/Hud.h"

#include "core/Config.h"
#include "core/Energy.h"
#include "core/PowerManager.h"
#include "core/Ui.h"

#include <cmath>
#include <cstdio>

namespace {

const float kMarginRight = 30.0f;
const float kTop = 22.0f;
const float kWidth = 300.0f;     // power list background width
const float kRowHeight = 30.0f;
const float kTextSize = 23.0f;
const float kBadgeSize = 18.0f;

// bottom center: [ENERGY] [■■■■■■■■□□□] [87]
void DrawEnergyBar() {
    const float barWidth = 420.0f, barHeight = 16.0f;
    float barLeft = (Ui::ScreenWidth() - barWidth) * 0.5f;
    float barRight = barLeft + barWidth;
    float barTop = Ui::ScreenHeight() - 46.0f, barBottom = barTop + barHeight;
    float fraction = Energy::Max() > 0.0f ? Energy::Current() / Energy::Max() : 0.0f;

    // soft dark backdrop behind the whole thing
    Ui::GradientH(barLeft - 140.0f, barTop - 10.0f, barLeft + barWidth * 0.5f, barBottom + 10.0f, CRGBA(0, 0, 0, 0), CRGBA(0, 0, 0, 140));
    Ui::GradientH(barLeft + barWidth * 0.5f, barTop - 10.0f, barRight + 140.0f, barBottom + 10.0f, CRGBA(0, 0, 0, 140), CRGBA(0, 0, 0, 0));

    // fill: blue, orange when low, red flash after a failed spend
    CRGBA top(90, 175, 255, 240), bottom(30, 90, 200, 240);
    if (fraction < 0.25f) {
        top = CRGBA(255, 175, 60, 240);
        bottom = CRGBA(200, 100, 20, 240);
    }
    if (Energy::IsFailFlashing()) {
        top = CRGBA(255, 70, 60, 255);
        bottom = CRGBA(170, 20, 15, 255);
    }
    Ui::Rect(barLeft - 2.0f, barTop - 2.0f, barRight + 2.0f, barBottom + 2.0f, CRGBA(0, 0, 0, 220));
    Ui::Rect(barLeft, barTop, barRight, barBottom, CRGBA(35, 35, 48, 220));
    if (fraction > 0.0f)
        Ui::GradientV(barLeft, barTop, barLeft + barWidth * fraction, barBottom, top, bottom);
    for (int i = 1; i < 4; ++i) { // tick marks every 25%
        float x = barLeft + barWidth * i / 4.0f;
        Ui::Rect(x, barTop, x + 1.5f, barBottom, CRGBA(0, 0, 0, 120));
    }

    float textY = barTop + (barHeight - 19.0f) * 0.5f - 1.0f;
    Ui::Text(barLeft - 14.0f, textY + 1.0f, "ENERGY", 17.0f, CRGBA(170, 210, 255, 255), Ui::Right, Ui::FontHeader);
    char value[8];
    std::snprintf(value, sizeof(value), "%d", (int)std::ceil(Energy::Current()));
    Ui::Text(barRight + 14.0f, textY, value, 20.0f, Ui::kText);
}
} // namespace

namespace Hud {

void Draw(PowerManager& powers) {
    float right = Ui::ScreenWidth() - kMarginRight;
    float y = kTop;

    Ui::Text(right, y, "Armageddon+", 34.0f, CRGBA(255, 160, 40, 255), Ui::Right, Ui::FontTitle);
    y += 40.0f;


    if (!powers.IsOverlayEnabled())
        return;

    // which powers to list: on, or bound to a key (so you can see your shortcuts)
    int rows = 0;
    for (size_t i = 0; i < powers.Count(); ++i)
        if (powers.At(i).IsActive() || powers.At(i).Key())
            ++rows;
    if (rows == 0)
        return;

    float top = y + 2.0f;
    Ui::GradientH(right - kWidth - 20.0f, top, right + 14.0f, top + rows * kRowHeight + 8.0f,
                  CRGBA(0, 0, 0, 0), CRGBA(0, 0, 0, 170));
    y = top + 4.0f;

    for (size_t i = 0; i < powers.Count(); ++i) {
        Power& power = powers.At(i);
        bool active = power.IsActive();
        if (!active && !power.Key())
            continue;

        float textY = y + (kRowHeight - kTextSize) * 0.5f - 1.0f;
        float x = right;
        // level: pips (or the level's name for named levels)
        if (active && power.LevelName(power.Level())) {
            Ui::Text(x, textY + 2.0f, power.LevelName(power.Level()), kTextSize * 0.8f, Ui::kActive, Ui::Right);
            x -= Ui::TextWidth(power.LevelName(power.Level()), kTextSize * 0.8f) + 10.0f;
        } else if (power.MaxLevel() > 1) {
            x -= Ui::Pips(x, y + kRowHeight * 0.5f - 4.5f, power.MaxLevel(), power.Level(), 9.0f,
                          active ? Ui::kActive : Ui::kTextDim) + 10.0f;
        } else if (active) {
            Ui::Rect(x - 9.0f, y + kRowHeight * 0.5f - 4.5f, x, y + kRowHeight * 0.5f + 4.5f, Ui::kActive);
            x -= 19.0f;
        }
        // name
        Ui::Text(x, textY, power.Name(), kTextSize, active ? Ui::kText : Ui::kTextDim, Ui::Right);
        x -= Ui::TextWidth(power.Name(), kTextSize) + 8.0f;
        // shortcut badge
        if (power.Key())
            Ui::KeyBadge(x, y + (kRowHeight - kBadgeSize) * 0.5f, Config::KeyName(power.Key()), kBadgeSize, true);
        y += kRowHeight;
    }
}

void DrawEnergy() {
    if (Energy::IsEnabled())
        DrawEnergyBar();
}

} // namespace Hud
