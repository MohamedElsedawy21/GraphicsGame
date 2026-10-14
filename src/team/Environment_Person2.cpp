// =====================================================================
//  OWNER: Person 2.  PLACEHOLDERS - replace with the detailed metro
//  station (entrance, stairs, roof, red "M" sign) and left street props
//  (streetlights, kiosk, trash bins). It must cover x = 0..160.
// =====================================================================
#include "team/Environment.h"
#include "engine/Renderer.h"
#include "engine/Text.h"
#include "shared/Palette.h"
#include "shared/Sizes.h"

void Environment::drawHomeBase(float) {
    float h = Layout::LANES_TOP - Layout::LANES_BOTTOM;
    Renderer::setOutline(2.0f, Palette::OUTLINE);
    Renderer::drawRect(0, Layout::LANES_BOTTOM, Layout::HOME_BASE_X, h, Palette::METRO_TILE);
    Renderer::drawCircle(80, Layout::LANES_TOP - 50, 30, Palette::METRO_RED);
    Renderer::setOutline(0.0f, Palette::OUTLINE);
    Text::drawCentered("M", 80, Layout::LANES_TOP - 64, 28, Palette::WHITE);
}

void Environment::drawLeftProps(float) {}
