// =====================================================================
//  OWNER: Person 4.  PLACEHOLDERS ONLY - replace each function body
//  with the real model. The placeholders just show the right size and
//  name so the rest of the game can be tested before the models exist.
// =====================================================================
#include "team/Models.h"
#include "engine/Renderer.h"
#include "engine/Text.h"
#include "shared/Palette.h"
#include "shared/Sizes.h"

namespace {
    void placeholder(float x, float y, float scale, const CharSize& size,
                     const Color& color, const char* name, bool hasWheels) {
        Renderer::pushTransform();
        Renderer::translate(x, y);
        Renderer::scale(scale, scale);

        Renderer::setOutline(2.0f, Palette::OUTLINE);
        Renderer::drawRoundedRect(-size.w / 2, 0, size.w, size.h, 8, color);
        if (hasWheels) {
            Renderer::drawCircle(-size.w * 0.3f, 0, 14, Palette::TIRE);
            Renderer::drawCircle( size.w * 0.3f, 0, 14, Palette::TIRE);
        }
        Renderer::setOutline(0.0f, Palette::OUTLINE);
        // A small triangle shows which way the model faces (right)
        Renderer::drawTriangle(size.w / 2 - 14, size.h - 10, size.w / 2 - 14, size.h - 26,
                               size.w / 2 - 4, size.h - 18, Palette::OUTLINE);
        Text::drawCentered(name, 0, size.h / 2 - 5, 10, Palette::OUTLINE);

        Renderer::popTransform();
    }
}

void Models::drawPedestrian(float x, float y, float scale, const AnimState&) {
    placeholder(x, y, scale, Sizes::PEDESTRIAN, Palette::SHIRT, "MELEE", false);
}
void Models::drawStoneThrower(float x, float y, float scale, const AnimState&) {
    placeholder(x, y, scale, Sizes::STONE_THROWER, Palette::GALABEYA, "RANGED", false);
}
void Models::drawBarrier(float x, float y, float scale, const AnimState&) {
    placeholder(x, y, scale, Sizes::BARRIER, Palette::BARRIER_WHITE, "DEFENSE", false);
}
void Models::drawTuktuk(float x, float y, float scale, const AnimState&) {
    placeholder(x, y, scale, Sizes::TUKTUK, Palette::TUKTUK_BODY, "TUKTUK", true);
}
void Models::drawMicrobus(float x, float y, float scale, const AnimState&) {
    placeholder(x, y, scale, Sizes::MICROBUS, Palette::MICROBUS_BODY, "MICROBUS", true);
}
void Models::drawCar(float x, float y, float scale, const AnimState&) {
    placeholder(x, y, scale, Sizes::CAR, Palette::CAR_BODY, "CAR", true);
}

void Models::drawStone(float x, float y, float) {
    Renderer::drawCircle(x, y, 5, Palette::STONE);
}
void Models::drawTire(float x, float y, float) {
    Renderer::drawCircle(x, y, 14, Palette::TIRE);
}
void Models::drawWatermelon(float x, float y, float) {
    Renderer::drawEllipse(x, y, 9, 7, Palette::WATERMELON);
}
