#include "ui/Hud.h"
#include "ui/CoinModel.h"
#include "engine/Renderer.h"
#include "engine/Text.h"
#include "engine/Input.h"
#include "engine/Audio.h"
#include "shared/Palette.h"
#include "team/Models.h"
#include <string>
#include <cmath>

namespace {
    // ===== Section A: card layout =====
    const float CARD_W = 80.0f;
    const float CARD_H = 86.0f;
    const float CARD_Y = Layout::HUD_Y + 7.0f;
    const float CARD_X[3] = { 18.0f, 108.0f, 198.0f };

    const PlayerType CARD_TYPES[3] = { PlayerType::Melee, PlayerType::Ranged, PlayerType::Defense };
    const char*      CARD_NAMES[3] = { "MELEE", "RANGED", "DEFENSE" };

    // ===== Section B: drawing pieces =====
    void drawPanel() {
        // Dark wooden band with gold trim
        Renderer::drawRect(0, Layout::HUD_Y, Layout::SCREEN_W, Layout::HUD_HEIGHT, Palette::UI_PANEL);
        Renderer::drawRect(0, Layout::HUD_Y, Layout::SCREEN_W, 6, Palette::COIN_GOLD_DARK);
        Renderer::drawRect(0, Layout::HUD_Y + 6, Layout::SCREEN_W, 2, Palette::OUTLINE);

        // A row of small diamonds: a simple Egyptian-style ornament
        for (float x = 310; x < 1000; x += 24) {
            if (x > 540 && x < 740) continue;   // leave the middle free for the pause hint
            Renderer::drawPolygon({ {x, Layout::HUD_Y + 14}, {x + 6, Layout::HUD_Y + 20},
                                    {x, Layout::HUD_Y + 26}, {x - 6, Layout::HUD_Y + 20} },
                                  Palette::UI_PANEL_LIGHT);
        }
    }

    void drawPadlock(float cx, float cy) {
        Renderer::setOutline(1.5f, Palette::OUTLINE);
        Renderer::drawLine(cx - 6, cy + 4, cx - 6, cy + 11, 3, Palette::METAL);
        Renderer::drawLine(cx + 6, cy + 4, cx + 6, cy + 11, 3, Palette::METAL);
        Renderer::drawLine(cx - 6, cy + 12, cx + 6, cy + 12, 3, Palette::METAL);
        Renderer::drawRoundedRect(cx - 10, cy - 8, 20, 14, 3, Palette::COIN_GOLD);
        Renderer::setOutline(0.0f, Palette::OUTLINE);
        Renderer::drawCircle(cx, cy, 2, Palette::OUTLINE);
    }

    void drawCard(int i, const World& world, float time) {
        PlayerType type = CARD_TYPES[i];
        bool unlocked = Hud::isCardUnlocked(world, type);
        bool selected = (world.selectedCard == type);
        bool hovered  = unlocked && Input::mouseIn(CARD_X[i], CARD_Y, CARD_W, CARD_H);

        float x = CARD_X[i];
        float y = CARD_Y + (hovered ? 3.0f : 0.0f);   // hover: the card lifts up

        // Locked cards are drawn completely in grayscale
        Renderer::setGrayscale(!unlocked);

        // Glow behind the selected card (pulses gently)
        if (selected) {
            float pulse = 0.35f + 0.15f * std::sin(time * 6.0f);
            Renderer::drawRoundedRect(x - 5, y - 5, CARD_W + 10, CARD_H + 10, 12,
                                      { 1.0f, 0.85f, 0.30f, pulse });
        }

        // Card body
        Renderer::setOutline(selected ? 3.5f : 2.0f, selected ? Palette::UI_SELECT : Palette::OUTLINE);
        Renderer::drawRoundedRect(x, y, CARD_W, CARD_H, 8,
                                  hovered ? Palette::UI_CARD_HOVER : Palette::UI_CARD);
        Renderer::setOutline(0.0f, Palette::OUTLINE);

        // Picture area with the character model
        Renderer::drawRoundedRect(x + 6, y + 22, CARD_W - 12, 48, 5, Palette::SKY_BOTTOM);
        AnimState idle;
        idle.time = time;
        idle.moving = false;
        float modelX = x + CARD_W / 2.0f;
        float modelY = y + 24.0f;
        if (type == PlayerType::Melee)   Models::drawPedestrian(modelX, modelY, 0.42f, idle);
        if (type == PlayerType::Ranged)  Models::drawStoneThrower(modelX, modelY, 0.42f, idle);
        if (type == PlayerType::Defense) Models::drawBarrier(modelX, modelY, 0.5f, idle);

        // Name at the top, cost at the bottom
        Text::drawCentered(CARD_NAMES[i], x + CARD_W / 2.0f, y + 74, 8, Palette::UI_TEXT_DARK);
        drawCoin(x + 22, y + 12, 7);
        Text::draw(std::to_string(cardCost(type)), x + 33, y + 6, 12, Palette::UI_TEXT_DARK);

        if (!unlocked) drawPadlock(x + CARD_W - 16, y + 50);

        Renderer::setGrayscale(false);
    }

    void drawCoinCounter(const World& world, float time) {
        float x = 1080.0f;
        float y = Layout::HUD_Y + 24.0f;
        Renderer::setOutline(2.0f, Palette::OUTLINE);
        Renderer::drawRoundedRect(x, y, 180, 52, 26, Palette::UI_PANEL_LIGHT);
        Renderer::setOutline(0.0f, Palette::OUTLINE);
        drawCoin(x + 28, y + 26, 20, time);
        Text::drawShadowed(std::to_string(world.coins.get()), x + 60, y + 13, 26,
                           Palette::COIN_GOLD, Palette::OUTLINE);
    }

    void drawWaveInfo(const World& world) {
        std::string wave = (world.currentWave == 0)
            ? "GET READY"
            : "WAVE " + std::to_string(world.currentWave) + " / 3";
        Text::drawShadowed(wave, 640 - Text::width(wave, 24) / 2.0f, Layout::HUD_Y + 44, 24,
                           Palette::UI_TEXT, Palette::OUTLINE);
        Text::drawCentered("ESC = PAUSE", 640, Layout::HUD_Y + 16, 9, Palette::UI_CARD);
    }
}

// ===== Section C: public functions =====
bool Hud::isCardUnlocked(const World& world, PlayerType type) {
    return world.coins.get() >= cardCost(type);
}

void Hud::handleInput(World& world) {
    if (!Input::mouseClicked()) return;
    for (int i = 0; i < 3; i++) {
        PlayerType type = CARD_TYPES[i];
        if (!isCardUnlocked(world, type)) continue;   // locked cards ignore the mouse
        if (Input::mouseIn(CARD_X[i], CARD_Y, CARD_W, CARD_H)) {
            // Click a card to select it; click it again to unselect it
            world.selectedCard = (world.selectedCard == type) ? PlayerType::None : type;
            Audio::play(Sfx::Click);
        }
    }
}

void Hud::update(World& world) {
    // If the coins dropped below the selected card's cost, that card is locked again
    if (world.selectedCard != PlayerType::None && !isCardUnlocked(world, world.selectedCard)) {
        world.selectedCard = PlayerType::None;
    }
}

void Hud::draw(const World& world, float time) {
    drawPanel();
    for (int i = 0; i < 3; i++) drawCard(i, world, time);
    drawWaveInfo(world);
    drawCoinCounter(world, time);
}
