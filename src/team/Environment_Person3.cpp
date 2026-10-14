// =====================================================================
//  OWNER: Person 3.  PLACEHOLDERS - replace with detailed versions
//  (balconies, shutters, signs, parked cars, a pedestrian bridge...).
//  Single plain shapes lose marks, so these must NOT stay as they are.
// =====================================================================
#include "team/Environment.h"
#include "engine/Renderer.h"
#include "shared/Palette.h"
#include "shared/Sizes.h"

void Environment::drawSkyAndBuildings(float) {
    // Sky: two bands from the palette
    Renderer::drawRect(0, Layout::LANES_TOP - 60, 1280, 160, Palette::SKY_TOP);
    // Placeholder buildings peeking above the top lane
    Renderer::drawRect(300, Layout::LANES_TOP - 60, 160, 60, Palette::BUILDING_1);
    Renderer::drawRect(520, Layout::LANES_TOP - 60, 120, 60, Palette::BUILDING_2);
    Renderer::drawRect(900, Layout::LANES_TOP - 60, 200, 60, Palette::BUILDING_3);
}

void Environment::drawRoad(float) {
    for (int lane = 0; lane < Layout::LANE_COUNT; lane++) {
        float bottom = Layout::laneBottom(lane);
        Color shade = (lane % 2 == 0) ? Palette::ASPHALT : Palette::ASPHALT_DARK;
        Renderer::drawRect(0, bottom, 1280, Layout::LANE_HEIGHT, shade);
        // Dashed lane line on top of every lane
        for (float x = 180; x < 1280; x += 60) {
            Renderer::drawRect(x, Layout::laneTop(lane) - 2, 30, 4, Palette::LANE_LINE);
        }
    }
}

void Environment::drawRightProps(float) {}

void Environment::drawForeground(float) {
    Renderer::drawRect(0, 0, 1280, Layout::LANES_BOTTOM, Palette::SIDEWALK);
    Renderer::drawRect(0, Layout::LANES_BOTTOM - 8, 1280, 8, Palette::CURB);
}
