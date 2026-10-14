#include "ui/Menus.h"
#include "ui/Button.h"
#include "ui/CoinModel.h"
#include "engine/Renderer.h"
#include "engine/Text.h"
#include "engine/Audio.h"
#include "shared/Palette.h"
#include <cmath>
#include <string>

namespace {
    // ===== Section A: the buttons of every menu =====
    Button startButton   (490, 360, 300, 58, "START");
    Button soundButton   (490, 284, 300, 58, "SOUND SETTINGS");
    Button exitButton    (490, 208, 300, 58, "EXIT");

    Button volDownButton (410, 270, 140, 54, "VOLUME -");
    Button muteButton    (570, 270, 140, 54, "MUTE");
    Button volUpButton   (730, 270, 140, 54, "VOLUME +");
    Button backButton    (540, 190, 200, 54, "BACK");

    Button continueButton(510, 384, 260, 58, "CONTINUE");
    Button restartButton (510, 308, 260, 58, "RESTART");
    Button mainMenuButton(510, 232, 260, 58, "MAIN MENU");

    Button endRestartButton (510, 290, 260, 58, "RESTART");
    Button endMainMenuButton(510, 214, 260, 58, "MAIN MENU");

    // ===== Section B: helpers =====
    Color mixColor(const Color& a, const Color& b, float t) {
        return { a.r + (b.r - a.r) * t, a.g + (b.g - a.g) * t, a.b + (b.b - a.b) * t, 1.0f };
    }

    void drawPanel(float x, float y, float w, float h) {
        Renderer::drawRoundedRect(x + 6, y - 8, w, h, 18, Palette::SHADOW);
        Renderer::setOutline(3.0f, Palette::OUTLINE);
        Renderer::drawRoundedRect(x, y, w, h, 18, Palette::UI_PANEL);
        Renderer::setOutline(0.0f, Palette::OUTLINE);
        // Gold inner border
        Renderer::setOutline(2.0f, Palette::COIN_GOLD_DARK);
        Renderer::drawRoundedRect(x + 10, y + 10, w - 20, h - 20, 12, Palette::UI_PANEL);
        Renderer::setOutline(0.0f, Palette::OUTLINE);
    }

    void dimScreen() {
        Renderer::drawRect(0, 0, 1280, 720, Palette::UI_DIM);
    }

    // ===== Section C: the Cairo-at-sunset backdrop behind the main menu =====
    void drawBuildingWindows(float x, float y, float w, float h, float time, int seed) {
        for (float wy = y + 14; wy < y + h - 14; wy += 22) {
            for (float wx = x + 8; wx < x + w - 14; wx += 18) {
                // Some windows are lit; they flicker slowly on and off
                int id = (int)(wx * 7 + wy * 13) + seed;
                bool lit = ((id + (int)(time * 0.5f)) % 5) < 2;
                Renderer::drawRect(wx, wy, 8, 11, lit ? Color{1.0f, 0.85f, 0.45f}
                                                      : Color{0.25f, 0.20f, 0.28f});
            }
        }
    }

    void drawBackdrop(float time) {
        // Sky: blend from dusky purple at the top to warm orange at the horizon
        Color top    {0.33f, 0.27f, 0.50f};
        Color middle {0.93f, 0.55f, 0.40f};
        Color bottom {0.99f, 0.85f, 0.60f};
        for (int i = 0; i < 26; i++) {
            float y = 200.0f + i * 20.0f;
            float t = i / 25.0f;   // 0 at the horizon, 1 at the top
            Color c = (t < 0.5f) ? mixColor(bottom, middle, t * 2.0f)
                                 : mixColor(middle, top, (t - 0.5f) * 2.0f);
            Renderer::drawRect(0, y, 1280, 21, c);
        }

        // Setting sun with a soft glow (on the right, so the buttons stay clear)
        Renderer::drawCircle(1090, 300, 110, {1.0f, 0.85f, 0.55f, 0.25f});
        Renderer::drawCircle(1090, 300, 80,  {1.0f, 0.90f, 0.60f, 0.35f});
        Renderer::drawCircle(1090, 300, 58, Palette::SUN);

        // Pyramids of Giza far away on the left (light side + shaded side)
        Color pyramidLight {0.86f, 0.62f, 0.45f};
        Color pyramidShade {0.70f, 0.48f, 0.38f};
        Renderer::drawTriangle(40, 200, 330, 200, 185, 380, pyramidLight);
        Renderer::drawTriangle(185, 380, 330, 200, 240, 200, pyramidShade);
        Renderer::drawTriangle(280, 200, 470, 200, 375, 315, pyramidLight);
        Renderer::drawTriangle(375, 315, 470, 200, 410, 200, pyramidShade);

        Color building {0.36f, 0.26f, 0.33f};
        Color building2{0.30f, 0.22f, 0.29f};

        // Cairo Tower: a tall narrow tower with a round top (left side)
        Renderer::drawTrapezoid(318, 200, 26, 16, 210, building2);
        Renderer::drawRect(312, 410, 38, 20, building);
        Renderer::drawRect(316, 430, 30, 7, building2);
        Renderer::drawLine(331, 437, 331, 475, 3, building2);

        // Mosque: dome + minaret + crescent (right of the buttons)
        Renderer::drawRect(830, 200, 120, 70, building);
        Renderer::drawEllipse(890, 270, 42, 38, building2);
        Renderer::drawRect(934, 200, 16, 190, building2);          // minaret shaft
        Renderer::drawRect(928, 330, 28, 8, building);             // minaret balcony
        Renderer::drawTriangle(934, 390, 950, 390, 942, 420, building2);
        Renderer::drawCircle(942, 432, 7, Palette::COIN_GOLD);     // crescent: gold circle...
        Renderer::drawCircle(945, 434, 6, mixColor(middle, top, 0.25f)); // ...with a sky bite

        // City skyline on the far right, with windows
        struct B { float x, w, h; };
        B blocks[] = { {975, 80, 150}, {1060, 95, 115}, {1160, 120, 180} };
        int seed = 0;
        for (const B& b : blocks) {
            Renderer::drawRect(b.x, 200, b.w, b.h, (seed % 2) ? building : building2);
            Renderer::drawRect(b.x - 3, 200 + b.h, b.w + 6, 6, building2);   // roof edge
            drawBuildingWindows(b.x, 200, b.w, b.h, time, seed * 31);
            seed++;
        }

        // Street at the bottom with moving lane dashes
        Renderer::drawRect(0, 0, 1280, 200, Palette::ASPHALT_DARK);
        Renderer::drawRect(0, 186, 1280, 14, Palette::CURB);
        float scroll = std::fmod(time * 120.0f, 80.0f);
        for (float x = -80 + scroll; x < 1280; x += 80) {
            Renderer::drawRect(x, 92, 44, 6, Palette::LANE_LINE);
        }
    }

    void drawTitle(float time) {
        float bob = std::sin(time * 2.0f) * 4.0f;
        std::string title = "CAIRO STREET DEFENSE";
        float size = 46.0f;
        Text::drawShadowed(title, 640 - Text::width(title, size) / 2.0f, 560 + bob, size,
                           Palette::UI_TEXT, Palette::OUTLINE);
        Text::drawCentered("PROTECT THE METRO STATION FROM CAIRO TRAFFIC!", 640, 520, 14,
                           Palette::UI_TEXT_DARK);
        drawCoin(640 - Text::width(title, size) / 2.0f - 40, 583 + bob, 22, time);
        drawCoin(640 + Text::width(title, size) / 2.0f + 40, 583 + bob, 22, time + 0.5f);
    }
}

// ===== Section D: main menu =====
MenuAction Menus::handleMain() {
    if (startButton.wasClicked()) return MenuAction::Start;
    if (soundButton.wasClicked()) return MenuAction::OpenSound;
    if (exitButton.wasClicked())  return MenuAction::Exit;
    return MenuAction::None;
}

void Menus::drawMain(float time) {
    drawBackdrop(time);
    drawTitle(time);
    startButton.draw();
    soundButton.draw();
    exitButton.draw();
}

// ===== Section E: sound settings =====
MenuAction Menus::handleSound() {
    if (volDownButton.wasClicked()) return MenuAction::VolumeDown;
    if (volUpButton.wasClicked())   return MenuAction::VolumeUp;
    if (muteButton.wasClicked())    return MenuAction::ToggleMute;
    if (backButton.wasClicked())    return MenuAction::Back;
    return MenuAction::None;
}

void Menus::drawSound(float time) {
    drawBackdrop(time);
    drawPanel(370, 160, 540, 360);
    Text::drawCentered("SOUND SETTINGS", 640, 450, 28, Palette::UI_TEXT);

    // Volume bar: 10 blocks, one per 10%
    int level = (int)std::round(Audio::volume() * 10.0f);
    bool muted = Audio::isMuted();
    for (int i = 0; i < 10; i++) {
        float bx = 455 + i * 38.0f;
        float bh = 14 + i * 3.0f;   // blocks grow taller, like a volume icon
        Color c = (i < level) ? (muted ? Palette::METAL : Palette::COIN_GOLD) : Palette::UI_PANEL_LIGHT;
        Renderer::setOutline(1.5f, Palette::OUTLINE);
        Renderer::drawRect(bx, 360, 30, bh, c);
        Renderer::setOutline(0.0f, Palette::OUTLINE);
    }
    std::string label = muted ? "MUTED" : "VOLUME " + std::to_string(level * 10) + "%";
    Text::drawCentered(label, 640, 334, 14, Palette::UI_TEXT);

    muteButton.label = muted ? "UNMUTE" : "MUTE";
    volDownButton.draw();
    muteButton.draw();
    volUpButton.draw();
    backButton.draw();
}

// ===== Section F: pause menu =====
MenuAction Menus::handlePause() {
    if (continueButton.wasClicked()) return MenuAction::Continue;
    if (restartButton.wasClicked())  return MenuAction::Restart;
    if (mainMenuButton.wasClicked()) return MenuAction::MainMenu;
    return MenuAction::None;
}

void Menus::drawPause() {
    dimScreen();
    drawPanel(450, 200, 380, 350);
    Text::drawCentered("PAUSED", 640, 470, 34, Palette::UI_TEXT);
    continueButton.draw();
    restartButton.draw();
    mainMenuButton.draw();
}

// ===== Section G: win / lose menu =====
MenuAction Menus::handleEnd() {
    if (endRestartButton.wasClicked())  return MenuAction::Restart;
    if (endMainMenuButton.wasClicked()) return MenuAction::MainMenu;
    return MenuAction::None;
}

void Menus::drawEnd(bool won, float time) {
    dimScreen();
    drawPanel(400, 180, 480, 380);

    if (won) {
        Text::drawShadowed("YOU WIN!", 640 - Text::width("YOU WIN!", 42) / 2.0f, 470, 42,
                           Palette::COIN_GOLD, Palette::OUTLINE);
        Text::drawCentered("THE METRO STATION IS SAFE.", 640, 420, 14, Palette::UI_TEXT);
        Text::drawCentered("CAIRO CAN GO HOME ON TIME.", 640, 396, 14, Palette::UI_TEXT);
        // Two spinning coins bouncing on each side of the title
        float bounce = std::fabs(std::sin(time * 3.0f)) * 12.0f;
        drawCoin(455, 488 + bounce, 18, time);
        drawCoin(825, 488 + bounce, 18, time + 0.4f);
    } else {
        Text::drawShadowed("GAME OVER", 640 - Text::width("GAME OVER", 42) / 2.0f, 470, 42,
                           Palette::UI_TEXT, Palette::OUTLINE);
        Text::drawCentered("A VEHICLE REACHED THE METRO STATION.", 640, 420, 12, Palette::UI_TEXT);
        Text::drawCentered("TRY AGAIN!", 640, 396, 14, Palette::UI_TEXT);
    }

    endRestartButton.draw();
    endMainMenuButton.draw();
}
