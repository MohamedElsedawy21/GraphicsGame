#include "ui/Button.h"
#include "engine/Renderer.h"
#include "engine/Text.h"
#include "engine/Input.h"
#include "engine/Audio.h"
#include "shared/Palette.h"

Button::Button(float x, float y, float w, float h, const std::string& label)
    : x(x), y(y), w(w), h(h), label(label) {}

bool Button::isHovered() const {
    return Input::mouseIn(x, y, w, h);
}

bool Button::wasClicked() const {
    if (isHovered() && Input::mouseClicked()) {
        Audio::play(Sfx::Click);
        return true;
    }
    return false;
}

void Button::draw() const {
    bool hover = isHovered();

    // Hover effect: the button lifts up a little and gets lighter
    float lift = hover ? 3.0f : 0.0f;

    // Shadow under the button
    Renderer::drawRoundedRect(x + 3, y - 4, w, h, 12, Palette::SHADOW);

    // The button itself
    Renderer::setOutline(2.5f, Palette::OUTLINE);
    Renderer::drawRoundedRect(x, y + lift, w, h, 12,
                              hover ? Palette::UI_BUTTON_HOVER : Palette::UI_BUTTON);
    Renderer::setOutline(0.0f, Palette::OUTLINE);

    // A thin gold line along the top: a small Egyptian-style detail
    Renderer::drawLine(x + 14, y + lift + h - 7, x + w - 14, y + lift + h - 7, 2.0f,
                       hover ? Palette::UI_SELECT : Palette::COIN_GOLD_DARK);

    // Label, centered
    float size = 18.0f;
    Text::drawCentered(label, x + w / 2.0f, y + lift + (h - size) / 2.0f - 2.0f, size,
                       Palette::UI_TEXT);
}
