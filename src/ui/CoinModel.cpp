#include "ui/CoinModel.h"
#include "engine/Renderer.h"
#include "shared/Palette.h"
#include <cmath>

void drawCoin(float cx, float cy, float radius, float spinTime) {
    // The coin is designed with a radius of 20 units, then scaled to the size asked for.
    float s = radius / 20.0f;

    // Spinning = squeezing the coin sideways with cos(), like a coin turning on a table
    float squeeze = 1.0f;
    if (spinTime != 0.0f) {
        squeeze = std::fabs(std::cos(spinTime * 3.0f));
        if (squeeze < 0.15f) squeeze = 0.15f;   // never vanish completely
    }

    Renderer::setDamage(0.0f);
    Renderer::pushTransform();
    Renderer::translate(cx, cy);
    Renderer::scale(s * squeeze, s);

    // 1. Edge: a darker coin peeking out below gives the coin some thickness
    Renderer::drawCircle(0, -1.5f, 20, Palette::COIN_GOLD_DARK);

    // 2. Brass outer ring
    Renderer::setOutline(1.5f, Palette::OUTLINE);
    Renderer::drawCircle(0, 0, 20, Palette::COIN_GOLD);
    Renderer::setOutline(0.0f, Palette::OUTLINE);

    // 3. Rim: a ring of small beads, like the milled edge of a real coin
    const int BEADS = 24;
    for (int i = 0; i < BEADS; i++) {
        float angle = 2.0f * 3.14159265f * i / BEADS;
        Renderer::drawCircle(17.5f * std::cos(angle), 17.5f * std::sin(angle), 1.1f,
                             Palette::COIN_GOLD_DARK);
    }

    // 4. Silver center (the real coin is bi-metal: brass outside, silver inside)
    Renderer::setOutline(1.0f, Palette::COIN_SILVER_DARK);
    Renderer::drawCircle(0, 0, 13, Palette::COIN_SILVER);
    Renderer::setOutline(0.0f, Palette::OUTLINE);

    // 5. Emblem: a pharaoh's mask (like the one on the Egyptian pound)
    //    headdress (nemes) = a wide trapezoid + a round top
    Renderer::drawTrapezoid(-9, -10, 18, 8, 14, Palette::COIN_GOLD);
    Renderer::drawCircle(0, 4, 4.5f, Palette::COIN_GOLD);
    //    blue stripes across the headdress
    Renderer::drawLine(-7.5f, -7, 7.5f, -7, 1.2f, Palette::COIN_BLUE);
    Renderer::drawLine(-6.0f, -3, 6.0f, -3, 1.2f, Palette::COIN_BLUE);
    Renderer::drawLine(-4.5f,  1, 4.5f,  1, 1.2f, Palette::COIN_BLUE);
    //    face and beard
    Renderer::drawEllipse(0, -2, 3.5f, 4.5f, Palette::COIN_GOLD_DARK);
    Renderer::drawRect(-1, -9.5f, 2, 3, Palette::COIN_BLUE);

    // 6. Shine: a soft white highlight on the top-left of the ring
    Renderer::drawEllipse(-9, 12, 4, 2, {1.0f, 1.0f, 1.0f, 0.45f});

    Renderer::popTransform();
}
