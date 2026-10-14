#include "ui/HealthBar.h"
#include "engine/Renderer.h"
#include "shared/Palette.h"

void drawHealthBar(float centerX, float y, int current, int max) {
    if (max <= 0) return;
    const float W = 60.0f;
    const float H = 8.0f;
    float left = centerX - W / 2.0f;

    // The bar must not turn red with its owner, so switch the damage tint off
    Renderer::setDamage(0.0f);

    // Dark background with an outline
    Renderer::setOutline(1.5f, Palette::OUTLINE);
    Renderer::drawRect(left, y, W, H, Palette::HEALTH_BACK);
    Renderer::setOutline(0.0f, Palette::OUTLINE);

    // Filled part: green -> yellow -> orange as health drops (never pure red)
    float fraction = (float)current / (float)max;
    if (fraction < 0.0f) fraction = 0.0f;
    Color fill = Palette::HEALTH_GOOD;
    if (fraction <= 0.6f) fill = {0.95f, 0.80f, 0.20f};
    if (fraction <= 0.3f) fill = {1.00f, 0.55f, 0.10f};
    Renderer::drawRect(left + 1, y + 1, (W - 2) * fraction, H - 2, fill);

    // Thin dividers: one section per life
    for (int i = 1; i < max; i++) {
        float lx = left + W * i / max;
        Renderer::drawLine(lx, y + 1, lx, y + H - 1, 1.0f, Palette::OUTLINE);
    }
}
