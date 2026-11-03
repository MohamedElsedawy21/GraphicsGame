
#include "team/Environment.h"
#include "engine/Renderer.h"
#include "engine/Text.h"
#include "shared/Palette.h"
#include "shared/Sizes.h"

// ============================================================
// SKY
// ============================================================

void Environment::drawSkyAndBuildings(float time) {

    Renderer::drawRect(
        0,
        Layout::LANES_TOP - 60,
        Layout::SCREEN_W,
        160,
        Palette::SKY_TOP
    );
}

// ============================================================
// ROAD - KEEP THE LANES CLEAR
// ============================================================

void Environment::drawRoad(float time) {

    for (int lane = 0; lane < Layout::LANE_COUNT; lane++) {

        float bottom = Layout::laneBottom(lane);

        Color shade = (lane % 2 == 0)
            ? Palette::ASPHALT
            : Palette::ASPHALT_DARK;

        Renderer::drawRect(
            0, bottom,
            Layout::SCREEN_W,
            Layout::LANE_HEIGHT,
            shade
        );

        // Dashed lane markings
        for (float x = 180; x < Layout::SCREEN_W; x += 60) {

            Renderer::drawRect(
                x,
                Layout::laneTop(lane) - 2,
                30, 4,
                Palette::LANE_LINE
            );
        }
    }
}

// ============================================================
// RIGHT PROPS
// ============================================================

void Environment::drawRightProps(float time) {
    // Street decorations are drawn in drawForeground().
}

// ============================================================
// SIDEWALK AND EGYPTIAN STREET DETAILS
// ============================================================

void Environment::drawForeground(float time) {

    // Sidewalk background
    Renderer::drawRect(
        0, 0,
        Layout::SCREEN_W,
        Layout::LANES_BOTTOM,
        Palette::SIDEWALK
    );

    // Curb
    Renderer::drawRect(
        0,
        Layout::LANES_BOTTOM - 8,
        Layout::SCREEN_W,
        8,
        Palette::CURB
    );

    // Sidewalk tile lines
    for (float x = 0; x < Layout::SCREEN_W; x += 100) {

        Renderer::drawLine(
            x, 0,
            x, Layout::LANES_BOTTOM - 8,
            2,
            Palette::BUILDING_3
        );
    }

    Renderer::setOutline(2.0f, Palette::OUTLINE);

    // ========================================================
    // EGYPTIAN STREET KIOSK
    // ========================================================

    // Main shop structure
    Renderer::drawRect(
        170, 10,
        170, 65,
        Palette::BUILDING_1
    );

    // Shop front
    Renderer::drawRect(
        185, 10,
        140, 48,
        Palette::BUILDING_2
    );

    // Display window
    Renderer::drawRect(
        195, 25,
        72, 30,
        Palette::WINDOW
    );

    // Window divider
    Renderer::drawLine(
        231, 25,
        231, 55,
        3,
        Palette::OUTLINE
    );

    // Shelves inside the shop
    Renderer::drawLine(
        199, 36,
        263, 36,
        2,
        Palette::METAL
    );

    Renderer::drawLine(
        199, 46,
        263, 46,
        2,
        Palette::METAL
    );

    // Products on display
    Renderer::drawRect(203, 38, 7, 7, Palette::AMBER);
    Renderer::drawRect(215, 38, 7, 7, Palette::BARRIER_RED);
    Renderer::drawRect(239, 38, 7, 7, Palette::SHUTTER);
    Renderer::drawRect(251, 38, 7, 7, Palette::AMBER);

    // Shop entrance
    Renderer::drawRect(
        278, 10,
        38, 48,
        Palette::BUILDING_3
    );

    // Door glass
    Renderer::drawRect(
        283, 15,
        28, 38,
        Palette::WINDOW
    );

    // Door handle
    Renderer::drawCircle(
        306, 34,
        2,
        Palette::AMBER
    );

    // Awning
    Renderer::drawRect(
        160, 75,
        190, 10,
        Palette::BARRIER_RED
    );

    // Awning stripes
    for (int i = 0; i < 6; i++) {

        Renderer::drawRect(
            165 + i * 30,
            75,
            12,
            10,
            Palette::WHITE
        );
    }

    // Signboard
    Renderer::drawRect(
        205, 87,
        100, 12,
        Palette::BUILDING_3
    );

    // KIOSK sign text
    Text::drawShadowed(
        "KIOSK",
        230, 88,
        8,
        Palette::WHITE,
        Palette::OUTLINE
    );

    // ========================================================
    // STREET LAMP
    // ========================================================

    Renderer::drawRect(
        1120, 10,
        7, 85,
        Palette::METAL
    );

    Renderer::drawLine(
        1123, 95,
        1085, 95,
        5,
        Palette::OUTLINE
    );

    Renderer::drawRoundedRect(
        1065, 90,
        35, 16,
        4,
        Palette::AMBER
    );

    // ========================================================
    // ROAD SIGN
    // ========================================================

    Renderer::drawRect(
        900, 10,
        5, 55,
        Palette::METAL
    );

    Renderer::drawRect(
        880, 55,
        45, 28,
        Palette::BARRIER_RED
    );

    Renderer::drawLine(
        885, 60,
        920, 78,
        3,
        Palette::WHITE
    );

    // ========================================================
    // ROADSIDE PLANT
    // ========================================================

    Renderer::drawRect(
        1020, 10,
        40, 22,
        Palette::BUILDING_2
    );

    Renderer::drawLine(
        1040, 32,
        1040, 55,
        3,
        Palette::SHUTTER
    );

    Renderer::drawEllipse(
        1025, 55,
        13, 9,
        Palette::SHUTTER
    );

    Renderer::drawEllipse(
        1040, 62,
        13, 10,
        Palette::SHUTTER
    );

    Renderer::drawEllipse(
        1055, 53,
        11, 9,
        Palette::SHUTTER
    );

    // ========================================================
    // TRASH BIN
    // ========================================================

    Renderer::drawRect(
        800, 10,
        30, 35,
        Palette::BUILDING_3
    );

    Renderer::drawRect(
        796, 45,
        38, 5,
        Palette::METAL
    );

    // ========================================================
    // STREET BENCH
    // ========================================================

    Renderer::drawRect(
        470, 35,
        95, 8,
        Palette::BUILDING_2
    );

    Renderer::drawRect(
        475, 43,
        7, 22,
        Palette::METAL
    );

    Renderer::drawRect(
        553, 43,
        7, 22,
        Palette::METAL
    );

    Renderer::setOutline(0.0f, Palette::OUTLINE);
}
