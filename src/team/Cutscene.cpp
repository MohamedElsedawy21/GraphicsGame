// =====================================================================
//  OWNER: Person 4.  PLACEHOLDER - replace with the real cutscene.
//  It shows a few seconds of moving placeholders so the
//  Main Menu -> Cutscene -> Playing flow can be tested now.
// =====================================================================
#include "team/Cutscene.h"
#include "team/Models.h"
#include "engine/Renderer.h"
#include "engine/Text.h"
#include "shared/Palette.h"

namespace {
    float timer = 0.0f;
    const float LENGTH = 5.0f;   // the real one must be 10-20 seconds
}

void Cutscene::start() {
    timer = 0.0f;
}

bool Cutscene::update(float dt) {
    timer += dt;
    return timer >= LENGTH;
}

void Cutscene::draw() {
    Renderer::drawRect(0, 0, 1280, 720, Palette::SKY_BOTTOM);
    Renderer::drawRect(0, 0, 1280, 200, Palette::ASPHALT);

    AnimState anim;
    anim.time = timer;
    Models::drawPedestrian(300, 200, 1.5f, anim);
    Models::drawTuktuk(1300 - timer * 220, 200, 1.5f, anim);

    Text::drawCentered("CUTSCENE PLACEHOLDER (PERSON 4)", 640, 600, 22, Palette::UI_TEXT_DARK);
}
