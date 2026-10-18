// =====================================================================
//  OWNER: Person 4  -  every character model.
//
//  Rules every model follows (from the team spec):
//    - origin (0, 0) = BOTTOM-CENTER, y = 0 is the ground, facing RIGHT
//    - built from many simple shapes (no single-shape parts)
//    - moving parts rotate around their own pivot (arms, legs, wheels, light)
//    - flat colors from Palette.h, dark outlines
//    - anim.damage tints the whole model red (setDamage), reset at the end
// =====================================================================
#include "team/Models.h"
#include "engine/Renderer.h"
#include "shared/Palette.h"
#include "shared/Sizes.h"
#include <cmath>
#include <vector>

namespace {
    const float PI = 3.14159265f;

    // ===== Section A: small helpers =====
    Color darker(const Color& c, float f = 0.75f) {
        return { c.r * f, c.g * f, c.b * f, c.a };
    }
    Color lighter(const Color& c, float f = 0.25f) {
        return { c.r + (1 - c.r) * f, c.g + (1 - c.g) * f, c.b + (1 - c.b) * f, c.a };
    }
    float clamp01(float v) { return v < 0 ? 0 : (v > 1 ? 1 : v); }
    float lerp(float a, float b, float t) { return a + (b - a) * t; }
    float ease(float t) { t = clamp01(t); return t * t * (3.0f - 2.0f * t); }

    void outlineOn(float w = 2.0f) { Renderer::setOutline(w, Palette::OUTLINE); }
    void outlineOff()              { Renderer::setOutline(0.0f, Palette::OUTLINE); }

    // Every model starts and ends the same way
    void beginModel(float x, float y, float scale, const AnimState& anim) {
        Renderer::pushTransform();
        Renderer::translate(x, y);
        Renderer::scale(scale, scale);
        Renderer::setDamage(anim.damage);
    }
    void endModel() {
        outlineOff();
        Renderer::setDamage(0.0f);
        Renderer::popTransform();
    }

    // A slice of a circle between two angles (degrees, under 180 apart = always convex)
    void drawCircleSegment(float cx, float cy, float r, float a0, float a1, const Color& c) {
        std::vector<Vec2> pts;
        const int steps = 14;
        for (int i = 0; i <= steps; i++) {
            float a = (a0 + (a1 - a0) * i / steps) * PI / 180.0f;
            pts.push_back({ cx + r * std::cos(a), cy + r * std::sin(a) });
        }
        Renderer::drawPolygon(pts, c);
    }

    // Cuts a polygon so only the part inside a rectangle is left
    // (used for the barrier's diagonal stripes). "Sutherland-Hodgman" clipping.
    std::vector<Vec2> clipEdge(const std::vector<Vec2>& in, bool useX, float value, bool keepBigger) {
        std::vector<Vec2> out;
        for (size_t i = 0; i < in.size(); i++) {
            Vec2 a = in[i];
            Vec2 b = in[(i + 1) % in.size()];
            float da = (useX ? a.x : a.y) - value;
            float db = (useX ? b.x : b.y) - value;
            if (!keepBigger) { da = -da; db = -db; }
            if (da >= 0) out.push_back(a);
            if ((da >= 0) != (db >= 0)) {
                float t = da / (da - db);
                out.push_back({ a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t });
            }
        }
        return out;
    }
    std::vector<Vec2> clipToRect(std::vector<Vec2> p, float x0, float y0, float x1, float y1) {
        p = clipEdge(p, true,  x0, true);  if (p.empty()) return p;
        p = clipEdge(p, true,  x1, false); if (p.empty()) return p;
        p = clipEdge(p, false, y0, true);  if (p.empty()) return p;
        return clipEdge(p, false, y1, false);
    }

    // ===== Section B: shared body parts =====
    void drawEye(float x, float y, float r) {
        Renderer::drawCircle(x, y, r, Palette::WHITE);
        Renderer::drawCircle(x + r * 0.35f, y, r * 0.5f, Palette::BLACK);   // looks forward
    }

    // Angry eyebrow: slopes DOWN toward the front of the face
    void drawAngryBrow(float x, float y, float w) {
        Renderer::drawLine(x - w / 2, y + w * 0.28f, x + w / 2, y - w * 0.12f, 2.2f, Palette::HAIR);
    }

    // Closed grumpy mouth, or a mouth that flaps open and shut while talking
    void drawMouth(float x, float y, float w, const AnimState& a) {
        if (a.talking) {
            float open = (std::sin(a.time * 24.0f) > 0.0f) ? 1.0f : 0.3f;
            Renderer::drawEllipse(x, y, w / 2, 0.8f + open * w * 0.45f, {0.35f, 0.08f, 0.10f});
        } else {
            Renderer::drawLine(x - w / 2, y - 0.7f, x + w / 2, y + 0.3f, 1.6f, Palette::BLACK);
        }
    }

    // A wheel that spins around its center: tire, tread blocks, hubcap, bolts
    void drawWheel(float cx, float cy, float r, float angle) {
        Renderer::pushTransform();
        Renderer::translate(cx, cy);
        Renderer::rotate(angle);

        outlineOn();
        Renderer::drawCircle(0, 0, r, Palette::TIRE);
        outlineOff();
        for (int i = 0; i < 10; i++) {                       // tread blocks
            Renderer::pushTransform();
            Renderer::rotate(i * 36.0f);
            Renderer::drawRect(-r * 0.12f, r * 0.78f, r * 0.24f, r * 0.17f, Palette::TREAD);
            Renderer::popTransform();
        }
        outlineOn(1.5f);
        Renderer::drawCircle(0, 0, r * 0.52f, Palette::HUBCAP);
        outlineOff();
        for (int i = 0; i < 5; i++) {                        // bolts
            float a = i * 72.0f * PI / 180.0f;
            Renderer::drawCircle(std::cos(a) * r * 0.3f, std::sin(a) * r * 0.3f, r * 0.07f,
                                 darker(Palette::HUBCAP, 0.6f));
        }
        Renderer::drawRect(-r * 0.05f, 0, r * 0.10f, r * 0.48f, darker(Palette::HUBCAP, 0.75f));
        Renderer::drawCircle(0, 0, r * 0.12f, darker(Palette::HUBCAP, 0.55f));

        Renderer::popTransform();
    }

    float wheelAngle(const AnimState& a, float speed = 600.0f) {
        return a.moving ? -a.time * speed : 0.0f;   // negative = clockwise = rolling forward
    }

    // A leg that swings from the hip; the shoe sticks out forward
    void drawLeg(float hipX, float hipY, float angle, const Color& trousers) {
        Renderer::pushTransform();
        Renderer::translate(hipX, hipY);
        Renderer::rotate(angle);
        outlineOn();
        Renderer::drawRect(-6, -36, 12, 38, trousers);
        Renderer::drawRoundedRect(-7, -40, 18, 8, 3, Palette::SHOE);
        outlineOff();
        Renderer::popTransform();
    }

    // Small motion streaks trailing behind a fast swing (the arm angle just passed)
    void drawSwingLines(float px, float py, float radius, float angleDeg) {
        for (int i = 1; i <= 3; i++) {
            float a = (angleDeg + i * 14.0f) * PI / 180.0f;
            float dx = std::sin(a), dy = -std::cos(a);   // direction the arm pointed
            Renderer::drawLine(px + dx * (radius - 6), py + dy * (radius - 6),
                               px + dx * (radius + 6), py + dy * (radius + 6),
                               2.0f, {1.0f, 1.0f, 1.0f, 0.6f - i * 0.15f});
        }
    }
}

// =====================================================================
//  1. MELEE: the angry pedestrian with his slipper (60 x 100)
// =====================================================================
void Models::drawPedestrian(float x, float y, float scale, const AnimState& anim) {
    beginModel(x, y, scale, anim);
    if (anim.ducking) Renderer::scale(1.12f, 0.55f);   // squash down while ducking

    float cycle = anim.moving ? std::sin(anim.time * 9.0f) : 0.0f;
    float bob   = anim.moving ? std::fabs(std::cos(anim.time * 9.0f)) * 2.0f
                              : std::sin(anim.time * 2.5f) * 0.8f;   // breathing
    Color skinShade = darker(Palette::SKIN, 0.85f);

    Renderer::drawEllipse(0, 1, 22, 3, Palette::SHADOW);

    // Back leg and back arm (darker: they are further away)
    drawLeg(-4, 40, -cycle * 24.0f, darker(Palette::TROUSERS, 0.8f));
    Renderer::pushTransform();
    Renderer::translate(-12, 72 + bob);
    Renderer::rotate(-cycle * 20.0f);
    outlineOn();
    Renderer::drawRect(-4, -22, 8, 22, skinShade);
    Renderer::drawCircle(0, -23, 4, skinShade);
    outlineOff();
    Renderer::popTransform();

    // Front leg
    drawLeg(5, 40, cycle * 24.0f, Palette::TROUSERS);

    // Torso: shirt with collar, pocket, button, belt
    outlineOn();
    Renderer::drawRoundedRect(-18, 40 + bob, 36, 36, 7, Palette::SHIRT);
    outlineOff();
    Renderer::drawRect(-17, 41 + bob, 34, 5, Palette::TROUSERS);
    Renderer::drawRect(-3, 41.5f + bob, 6, 4, Palette::COIN_GOLD_DARK);              // buckle
    Renderer::drawTriangle(-6, 75 + bob, 6, 75 + bob, 0, 67 + bob, darker(Palette::SHIRT, 0.75f));
    Renderer::drawRect(4, 58 + bob, 9, 7, darker(Palette::SHIRT, 0.88f));            // pocket
    Renderer::drawCircle(0, 60 + bob, 1.3f, Palette::WHITE);
    Renderer::drawCircle(0, 53 + bob, 1.3f, Palette::WHITE);

    // Head: neck, face, hair, ear, angry eye and brow, nose, mustache, mouth
    float hy = 88 + bob;
    Renderer::drawRect(-4, 74 + bob, 8, 5, skinShade);
    outlineOn();
    Renderer::drawCircle(1, hy, 11, Palette::SKIN);
    outlineOff();
    drawCircleSegment(1, hy, 11.6f, 28, 200, Palette::HAIR);
    Renderer::drawCircle(-3, hy - 1, 2.6f, skinShade);
    drawEye(6, hy + 1, 2.6f);
    drawAngryBrow(6, hy + 4.5f, 7);
    Renderer::drawTriangle(10, hy + 1, 14.5f, hy - 3, 10, hy - 3, skinShade);
    Renderer::drawEllipse(8, hy - 4.5f, 4.5f, 1.6f, Palette::HAIR);
    drawMouth(8, hy - 7.2f, 4.5f, anim);

    // Front arm with the slipper. Pivot = shoulder (14, 72).
    // Attack: wind the arm BACK behind the head (0 -> -160 degrees, clockwise),
    // then smash over the top and down in front (-160 -> -300, i.e. pointing forward).
    float armAngle;
    float p = anim.attack;
    if (p > 0.0f) {
        armAngle = (p < 0.45f) ? lerp(0, -160, ease(p / 0.45f))
                               : lerp(-160, -300, ease((p - 0.45f) / 0.55f));
    } else {
        armAngle = anim.moving ? cycle * 22.0f : 6.0f + std::sin(anim.time * 2.5f) * 3.0f;
    }
    if (p > 0.5f && p < 0.85f) drawSwingLines(14, 72 + bob, 36, armAngle);

    Renderer::pushTransform();
    Renderer::translate(14, 72 + bob);
    Renderer::rotate(armAngle);
    outlineOn();
    Renderer::drawRect(-5, -11, 10, 11, Palette::SHIRT);     // rolled-up sleeve
    Renderer::drawRect(-4, -24, 8, 14, Palette::SKIN);       // forearm
    Renderer::drawCircle(0, -25, 4.5f, Palette::SKIN);       // hand
    Renderer::drawEllipse(0, -36, 5.5f, 12, Palette::SLIPPER);  // the slipper (24 x 10 ish)
    outlineOff();
    Renderer::drawEllipse(0, -37, 3.5f, 9, lighter(Palette::SLIPPER, 0.3f));      // footbed
    Renderer::drawRect(-5.5f, -40, 11, 4, darker(Palette::SLIPPER, 0.7f));         // strap
    Renderer::popTransform();

    endModel();
}

// =====================================================================
//  2. RANGED: the stone thrower in a galabeya (60 x 100)
// =====================================================================
void Models::drawStoneThrower(float x, float y, float scale, const AnimState& anim) {
    beginModel(x, y, scale, anim);
    if (anim.ducking) Renderer::scale(1.12f, 0.55f);

    float cycle = anim.moving ? std::sin(anim.time * 8.0f) : 0.0f;
    float bob   = anim.moving ? std::fabs(std::cos(anim.time * 8.0f)) * 1.6f
                              : std::sin(anim.time * 2.2f) * 0.8f;
    Color robe = Palette::GALABEYA;
    Color robeDark = darker(robe, 0.8f);
    Color skinShade = darker(Palette::SKIN, 0.85f);

    Renderer::drawEllipse(0, 1, 22, 3, Palette::SHADOW);

    // Sandals peeking out under the robe (they shuffle while walking)
    outlineOn(1.5f);
    Renderer::drawRoundedRect(-14 + cycle * 5, 0, 14, 6, 3, Palette::SHOE);
    Renderer::drawRoundedRect(  0 - cycle * 5, 0, 14, 6, 3, Palette::SHOE);
    outlineOff();

    // Back arm
    Renderer::pushTransform();
    Renderer::translate(-11, 73 + bob);
    Renderer::rotate(-cycle * 12.0f);
    outlineOn();
    Renderer::drawRect(-4, -24, 8, 24, robeDark);
    Renderer::drawCircle(0, -25, 4, skinShade);
    outlineOff();
    Renderer::popTransform();

    // Robe (trapezoid: 40 wide at the hem, 30 at the shoulders) + upper torso
    outlineOn();
    Renderer::drawTrapezoid(-20, 4, 40, 30, 58 + bob, robe);
    Renderer::drawRoundedRect(-15, 60 + bob, 30, 18, 6, robe);
    outlineOff();
    Renderer::drawLine(-19, 10, 19, 10, 2.0f, robeDark);                 // hem band
    Renderer::drawLine(3, 76 + bob, 3, 52 + bob, 1.5f, robeDark);         // front opening
    Renderer::drawCircle(3, 71 + bob, 1.2f, robeDark);
    Renderer::drawCircle(3, 65 + bob, 1.2f, robeDark);

    // Stone pouch on a strap, at the hip
    Renderer::drawLine(10, 77 + bob, -12, 46 + bob, 2.5f, darker(Palette::POUCH, 0.8f));
    outlineOn(1.5f);
    Renderer::drawRoundedRect(-22, 29 + bob, 16, 18, 6, Palette::POUCH);
    Renderer::drawCircle(-18, 47 + bob, 3.2f, Palette::STONE);
    Renderer::drawCircle(-12.5f, 47.5f + bob, 3.0f, Palette::STONE);
    outlineOff();
    Renderer::drawRect(-21, 42 + bob, 14, 2.5f, darker(Palette::POUCH, 0.7f));

    // Head: face, beard, head wrap with a tail, angry eye and brow
    float hy = 88 + bob;
    Renderer::drawRect(-4, 75 + bob, 8, 5, skinShade);
    outlineOn();
    Renderer::drawCircle(1, hy, 11, Palette::SKIN);
    outlineOff();
    drawCircleSegment(1, hy, 11.3f, 205, 335, Palette::BEARD);
    Renderer::drawTriangle(-9, hy + 3, -17, hy - 5, -8, hy - 3, Palette::HEAD_WRAP);   // wrap tail
    outlineOn(1.5f);
    drawCircleSegment(1, hy + 1, 12, 8, 172, Palette::HEAD_WRAP);
    Renderer::drawEllipse(1, hy + 3.5f, 12.5f, 3, darker(Palette::HEAD_WRAP, 0.9f));
    outlineOff();
    drawEye(6, hy + 0.5f, 2.5f);
    drawAngryBrow(6, hy + 3.6f, 7);
    Renderer::drawTriangle(10, hy, 14, hy - 3.5f, 10, hy - 3.5f, skinShade);
    drawMouth(8, hy - 6, 4, anim);

    // Throwing arm. Pivot = shoulder (12, 74).
    // Attack: wind the arm back (360 -> 215 degrees), then over the top and forward (215 -> 55).
    float armAngle;
    float p = anim.attack;
    if (p > 0.0f) {
        armAngle = (p < 0.5f) ? lerp(360, 215, ease(p / 0.5f))
                              : lerp(215, 55, ease((p - 0.5f) / 0.5f));
    } else {
        armAngle = anim.moving ? cycle * 15.0f : 5.0f + std::sin(anim.time * 2.2f) * 3.0f;
    }
    if (p > 0.55f && p < 0.85f) drawSwingLines(12, 74 + bob, 34, armAngle);

    Renderer::pushTransform();
    Renderer::translate(12, 74 + bob);
    Renderer::rotate(armAngle);
    outlineOn();
    Renderer::drawRect(-5, -26, 10, 26, robe);               // sleeve
    Renderer::drawCircle(0, -27, 4.5f, Palette::SKIN);       // hand
    if (p < 0.62f) {                                          // stone still in the hand
        outlineOn(1.5f);
        Renderer::drawCircle(0, -31, 5, Palette::STONE);
        outlineOff();
        Renderer::drawCircle(-1.5f, -29.5f, 1.4f, lighter(Palette::STONE, 0.4f));
    }
    outlineOff();
    Renderer::popTransform();

    endModel();
}

// =====================================================================
//  3. DEFENSE: the striped traffic barrier (70 x 80)
// =====================================================================
void Models::drawBarrier(float x, float y, float scale, const AnimState& anim) {
    beginModel(x, y, scale, anim);
    Color metalDark = darker(Palette::METAL, 0.7f);

    Renderer::drawEllipse(0, 1, 36, 3, Palette::SHADOW);

    // Slightly angled support legs + wide feet
    outlineOn();
    Renderer::drawPolygon({ {-28, 10}, {-20, 10}, {-13, 34}, {-21, 34} }, Palette::METAL);
    Renderer::drawPolygon({ { 20, 10}, { 28, 10}, { 21, 34}, { 13, 34} }, Palette::METAL);
    Renderer::drawRoundedRect(-31, 0, 14, 10, 3, metalDark);
    Renderer::drawRoundedRect( 17, 0, 14, 10, 3, metalDark);
    outlineOff();

    // Main board: white with 5 diagonal red stripes, cut to fit the board
    Renderer::drawRect(-33, 30, 66, 34, Palette::BARRIER_WHITE);
    for (int i = 0; i < 5; i++) {
        float x0 = -40.0f + i * 16.0f;
        std::vector<Vec2> stripe = clipToRect({ {x0, 30}, {x0 + 8, 30}, {x0 + 22, 64}, {x0 + 14, 64} },
                                              -33, 30, 33, 64);
        if (stripe.size() >= 3) Renderer::drawPolygon(stripe, Palette::BARRIER_RED);
    }
    // Board border and bolts
    Renderer::drawLine(-33, 30, 33, 30, 2, Palette::OUTLINE);
    Renderer::drawLine(-33, 64, 33, 64, 2, Palette::OUTLINE);
    Renderer::drawLine(-33, 29, -33, 65, 2, Palette::OUTLINE);
    Renderer::drawLine( 33, 29,  33, 65, 2, Palette::OUTLINE);
    Renderer::drawCircle(-28, 35, 1.8f, metalDark);
    Renderer::drawCircle( 28, 35, 1.8f, metalDark);
    Renderer::drawCircle(-28, 59, 1.8f, metalDark);
    Renderer::drawCircle( 28, 59, 1.8f, metalDark);

    // Top rail + warning light housing
    outlineOn();
    Renderer::drawRect(-35, 64, 70, 6, Palette::METAL);
    Renderer::drawRoundedRect(-7, 70, 14, 10, 4, metalDark);
    outlineOff();

    // Flashing amber light (on for half a second, off for half a second)
    bool on = std::fmod(anim.time, 1.0f) < 0.5f;
    if (on) Renderer::drawCircle(0, 76, 13, {1.0f, 0.75f, 0.2f, 0.30f});
    Renderer::drawEllipse(0, 75.5f, 5, 4, on ? Palette::AMBER : darker(Palette::AMBER, 0.55f));
    Renderer::drawCircle(-1.8f, 77, 1.2f, {1, 1, 1, on ? 0.9f : 0.4f});

    endModel();
}

// =====================================================================
//  4. HARD ENEMY: the rust-orange car (150 x 80)
// =====================================================================
void Models::drawCar(float x, float y, float scale, const AnimState& anim) {
    beginModel(x, y, scale, anim);
    Color body = Palette::CAR_BODY;
    Color bodyDark = darker(body, 0.75f);
    float wheel = wheelAngle(anim, 650.0f);
    float bounce = anim.moving ? std::sin(anim.time * 20.0f) * 0.8f : 0.0f;

    Renderer::drawEllipse(0, 2, 74, 4, Palette::SHADOW);

    // Exhaust puffs from the back while driving
    if (anim.moving) {
        for (int i = 0; i < 3; i++) {
            float age = std::fmod(anim.time * 2.0f + i / 3.0f, 1.0f);
            Renderer::drawCircle(-80 - age * 25, 20 + age * 8, 3 + age * 6,
                                 {0.6f, 0.6f, 0.6f, 0.5f * (1.0f - age)});
        }
    }

    Renderer::pushTransform();
    Renderer::translate(0, bounce);

    // Cabin (trapezoid, shifted back) and roof
    outlineOn();
    Renderer::drawTrapezoid(-46, 42, 76, 64, 28, body);
    Renderer::drawRoundedRect(-41, 69, 66, 7, 3, bodyDark);
    // Windows: rear, side, windshield
    outlineOn(1.5f);
    Renderer::drawPolygon({ {-40, 49}, {-25, 49}, {-25, 65}, {-37, 65} }, Palette::GLASS);
    Renderer::drawRect(-21, 48, 19, 18, Palette::GLASS);
    Renderer::drawPolygon({ {3, 48}, {27, 48}, {23.5f, 66}, {3, 66} }, Palette::GLASS);
    outlineOff();

    // Angry driver in the side window
    float hx = -11, hy = 56;
    Renderer::drawCircle(hx, hy, 6.5f, Palette::SKIN);
    drawCircleSegment(hx, hy, 6.8f, 20, 190, Palette::HAIR);
    drawEye(hx + 3.5f, hy + 0.5f, 1.8f);
    Renderer::drawLine(hx + 1, hy + 4, hx + 6.5f, hy + 2, 1.8f, Palette::HAIR);
    Renderer::drawRect(hx + 1.5f, hy - 4.5f, 5, 2, Palette::WHITE);   // gritted teeth
    // Glass shine
    Renderer::drawLine(7, 51, 12, 62, 2, {1, 1, 1, 0.35f});
    Renderer::drawLine(-18, 51, -14, 60, 2, {1, 1, 1, 0.30f});

    // Lower body, sloping hood wedge, trunk
    outlineOn();
    Renderer::drawRoundedRect(-75, 16, 150, 28, 6, body);
    Renderer::drawPolygon({ {28, 44}, {75, 44}, {75, 47}, {28, 54} }, body);
    Renderer::drawPolygon({ {-75, 44}, {-45, 44}, {-45, 52}, {-72, 52} }, body);
    outlineOff();

    // Details: side stripe, doors, handles, bumper
    Renderer::drawRect(-74, 29, 148, 3, bodyDark);
    Renderer::drawLine(-20, 19, -20, 43, 1.5f, bodyDark);
    Renderer::drawLine(4, 19, 4, 43, 1.5f, bodyDark);
    Renderer::drawRoundedRect(-15, 35, 7, 2.5f, 1, Palette::METAL);
    Renderer::drawRoundedRect(9, 35, 7, 2.5f, 1, Palette::METAL);
    Renderer::drawRect(-74, 16, 148, 5, darker(Palette::METAL, 0.6f));

    // Lights (amber at the back: red is only for damage) + headlight beam
    Renderer::drawTriangle(76, 36, 112, 26, 112, 46, {1.0f, 1.0f, 0.7f, 0.15f});
    outlineOn(1.5f);
    Renderer::drawRoundedRect(66, 32, 10, 8, 2, Palette::HEADLIGHT);
    Renderer::drawRect(-76, 32, 6, 8, Palette::AMBER);
    // Pointed bull bar
    Renderer::drawPolygon({ {73, 18}, {81, 22}, {85, 29}, {81, 36}, {73, 40} }, Palette::METAL);
    outlineOff();
    Renderer::drawLine(77, 22, 77, 37, 1.5f, darker(Palette::METAL, 0.6f));

    // Dark wheel arches
    drawCircleSegment(-44, 18, 21, 0, 180, darker(body, 0.35f));
    drawCircleSegment( 46, 18, 21, 0, 180, darker(body, 0.35f));
    Renderer::popTransform();

    drawWheel(-44, 18, 18, wheel);
    drawWheel( 46, 18, 18, wheel);
    endModel();
}

// =====================================================================
//  5. EASY ENEMY: the yellow tuk-tuk (110 x 105)
// =====================================================================
void Models::drawTuktuk(float x, float y, float scale, const AnimState& anim) {
    beginModel(x, y, scale, anim);
    Color body = Palette::TUKTUK_BODY;
    Color bodyDark = darker(body, 0.75f);
    float wheel = wheelAngle(anim, 700.0f);
    float bob = anim.moving ? std::sin(anim.time * 14.0f) * 1.2f : 0.0f;

    Renderer::drawEllipse(0, 2, 56, 4, Palette::SHADOW);

    // Third wheel on the far side (behind everything, a bit darker)
    drawWheel(-22, 15, 14, wheel);
    Renderer::drawCircle(-22, 15, 14.5f, {0, 0, 0, 0.35f});

    Renderer::pushTransform();
    Renderer::translate(0, bob);

    // Canopy poles
    Renderer::drawLine(-52, 44, -52, 86, 4, darker(Palette::METAL, 0.6f));
    Renderer::drawLine(30, 44, 30, 86, 4, darker(Palette::METAL, 0.6f));

    // Open passenger cabin: back panel, seat
    outlineOn();
    Renderer::drawRect(-55, 44, 10, 40, bodyDark);
    Renderer::drawRect(-45, 50, 8, 24, Palette::SEAT);
    Renderer::drawRoundedRect(-45, 48, 34, 9, 3, Palette::SEAT);

    // Driver: body, head, arm on the handlebar
    Renderer::drawRoundedRect(19, 56, 15, 20, 4, Palette::HEAD_WRAP);
    Renderer::drawCircle(28, 79, 9, Palette::SKIN);
    outlineOff();
    drawCircleSegment(28, 79, 9.4f, 30, 195, Palette::HAIR);
    drawEye(32, 80, 2.2f);
    drawAngryBrow(32, 83.5f, 6);
    Renderer::drawEllipse(33, 75.5f, 3.5f, 1.3f, Palette::HAIR);
    drawMouth(33, 73, 3.5f, anim);
    Renderer::drawLine(30, 67, 45, 57, 4, Palette::SKIN);

    // Handlebar on its pole
    Renderer::drawLine(48, 44, 46, 57, 3, Palette::METAL);
    outlineOn(1.5f);
    Renderer::drawRoundedRect(40, 55, 14, 4, 2, darker(Palette::METAL, 0.6f));
    outlineOff();

    // Side rails of the open cabin
    Renderer::drawLine(-10, 44, -10, 84, 3, Palette::METAL);
    Renderer::drawLine(-44, 64, -10, 64, 2.5f, Palette::METAL);

    // Chassis: body + rounded nose
    outlineOn();
    Renderer::drawRoundedRect(-55, 14, 85, 30, 8, body);
    Renderer::drawPolygon({ {28, 14}, {50, 14}, {56, 28}, {48, 44}, {28, 44} }, body);
    outlineOff();
    // Painted checker stripe (tuk-tuks love decorations)
    for (int i = 0; i < 8; i++) {
        Color c = (i % 2 == 0) ? Palette::COIN_BLUE : bodyDark;
        Renderer::drawRect(-50 + i * 9.0f, 30, 9, 5, c);
    }
    // Front mudguard and headlight
    outlineOn(1.5f);
    drawCircleSegment(40, 15, 17, 0, 180, bodyDark);
    Renderer::drawCircle(54, 35, 5, Palette::HEADLIGHT);
    outlineOff();

    // Canopy roof (slightly curved) with colorful fringe
    outlineOn();
    Renderer::drawPolygon({ {-58, 84}, {34, 84}, {36, 89}, {26, 94}, {-46, 94}, {-58, 90} },
                          Palette::TUKTUK_CANOPY);
    outlineOff();
    for (int i = 0; i < 11; i++) {
        float fx = -55 + i * 8.0f;
        Color c = (i % 2 == 0) ? Palette::AMBER : Palette::COIN_BLUE;
        Renderer::drawTriangle(fx, 84, fx + 6, 84, fx + 3, 78, c);
    }

    // Roof stack: a crate and a watermelon (the things it throws)
    outlineOn(1.5f);
    Renderer::drawRect(-34, 94, 13, 11, Palette::CRATE);
    Renderer::drawEllipse(-12, 99.5f, 8, 5.5f, Palette::WATERMELON);
    outlineOff();
    Renderer::drawLine(-34, 99.5f, -21, 99.5f, 1.2f, darker(Palette::CRATE, 0.6f));
    Renderer::drawLine(-27.5f, 94, -27.5f, 105, 1.2f, darker(Palette::CRATE, 0.6f));
    Renderer::drawLine(-17, 97, -7, 97, 1.3f, darker(Palette::WATERMELON, 0.6f));
    Renderer::drawLine(-18, 101, -6, 101, 1.3f, darker(Palette::WATERMELON, 0.6f));

    Renderer::popTransform();

    drawWheel(-30, 15, 15, wheel);
    drawWheel( 40, 15, 14, wheel);
    endModel();
}

// =====================================================================
//  6. MEDIUM ENEMY: the white microbus (170 x 110)
// =====================================================================
void Models::drawMicrobus(float x, float y, float scale, const AnimState& anim) {
    beginModel(x, y, scale, anim);
    Color body = Palette::MICROBUS_BODY;
    Color bodyDark = darker(body, 0.8f);
    float wheel = wheelAngle(anim, 520.0f);
    float bounce = anim.moving ? std::sin(anim.time * 16.0f) * 0.8f : 0.0f;

    Renderer::drawEllipse(0, 2, 84, 4, Palette::SHADOW);

    Renderer::pushTransform();
    Renderer::translate(0, bounce);

    // Luggage on the roof rack
    outlineOn(1.5f);
    Renderer::drawRoundedRect(-52, 100, 30, 10, 3, {0.55f, 0.35f, 0.25f});
    Renderer::drawRoundedRect(-18, 100, 22, 8, 3, {0.30f, 0.45f, 0.60f});
    Renderer::drawRect(10, 100, 18, 10, Palette::CRATE);
    outlineOff();

    // Upper cabin (narrower, slanted windshield) and lower body
    outlineOn();
    Renderer::drawPolygon({ {-82, 60}, {78, 60}, {68, 98}, {-80, 98} }, body);
    Renderer::drawRoundedRect(-85, 18, 170, 44, 6, body);
    outlineOff();
    Renderer::drawRect(-84, 40, 168, 6, Palette::MICROBUS_STRIPE);
    Renderer::drawRect(-80, 92, 146, 3, Palette::MICROBUS_STRIPE);

    // Roof rack: rail + posts
    Renderer::drawRect(-60, 104, 120, 2.5f, Palette::METAL);
    for (int i = 0; i < 4; i++) Renderer::drawRect(-58 + i * 38.0f, 98, 2.5f, 8, Palette::METAL);

    // Five side windows + windshield
    outlineOn(1.5f);
    for (int i = 0; i < 5; i++) Renderer::drawRect(-76 + i * 28.0f, 68, 22, 22, Palette::GLASS);
    Renderer::drawPolygon({ {61, 66}, {75, 66}, {69, 92}, {61, 92} }, Palette::GLASS);
    outlineOff();
    // Passenger silhouettes and the angry driver
    int seats[3] = { 0, 2, 3 };
    for (int s : seats) {
        float px = -65 + s * 28.0f;
        Renderer::drawCircle(px, 76, 6, {0.22f, 0.18f, 0.20f, 0.75f});
        Renderer::drawRect(px - 7, 68, 14, 4, {0.22f, 0.18f, 0.20f, 0.75f});
    }
    Renderer::drawCircle(66, 79, 5.5f, Palette::SKIN);
    drawCircleSegment(66, 79, 5.8f, 30, 190, Palette::HAIR);
    Renderer::drawLine(66, 82.5f, 71, 80.5f, 1.6f, Palette::HAIR);
    Renderer::drawCircle(69, 79.5f, 1.1f, Palette::BLACK);
    for (int i = 0; i < 5; i++) {
        float wx = -76 + i * 28.0f;
        Renderer::drawLine(wx + 4, 72, wx + 9, 84, 1.8f, {1, 1, 1, 0.3f});   // shine
    }

    // Sliding door: outline, rail and handle
    Renderer::drawLine(0, 20, 0, 60, 1.5f, bodyDark);
    Renderer::drawLine(36, 20, 36, 60, 1.5f, bodyDark);
    Renderer::drawLine(-2, 58, 40, 58, 1.5f, bodyDark);
    Renderer::drawRoundedRect(28, 49, 6, 2.5f, 1, Palette::METAL);

    // Bumpers, headlight, taillight, mirror
    outlineOn(1.5f);
    Renderer::drawRect(80, 18, 10, 8, darker(Palette::METAL, 0.7f));
    Renderer::drawRect(-90, 18, 10, 8, darker(Palette::METAL, 0.7f));
    Renderer::drawRect(78, 35, 8, 10, Palette::HEADLIGHT);
    Renderer::drawRect(-87, 35, 5, 9, Palette::AMBER);
    Renderer::drawRect(80, 70, 4, 8, Palette::OUTLINE);
    outlineOff();
    Renderer::drawLine(74, 72, 81, 74, 2, Palette::OUTLINE);

    // Wheel arches
    drawCircleSegment(-52, 20, 23, 0, 180, darker(body, 0.35f));
    drawCircleSegment( 52, 20, 23, 0, 180, darker(body, 0.35f));
    Renderer::popTransform();

    drawWheel(-52, 20, 20, wheel);
    drawWheel( 52, 20, 20, wheel);
    endModel();
}

// =====================================================================
//  CUTSCENE ONLY: the big city bus (320 x 150)
// =====================================================================
void Models::drawBus(float x, float y, float scale, const AnimState& anim) {
    beginModel(x, y, scale, anim);
    Color body = Palette::BUS_BODY;
    float wheel = wheelAngle(anim, 420.0f);
    float bounce = anim.moving ? std::sin(anim.time * 12.0f) * 1.0f : 0.0f;

    Renderer::drawEllipse(0, 2, 160, 5, Palette::SHADOW);

    Renderer::pushTransform();
    Renderer::translate(0, bounce);

    // Body: green lower half, cream upper half, yellow stripe between
    outlineOn(2.5f);
    Renderer::drawRoundedRect(-160, 22, 320, 120, 12, body);
    outlineOff();
    Renderer::drawRect(-157, 74, 314, 64, Palette::BUS_TOP);
    Renderer::drawRect(-157, 68, 314, 6, Palette::AMBER);

    // Destination sign (LED dots) on top
    outlineOn(1.5f);
    Renderer::drawRect(-60, 126, 120, 10, Palette::BLACK);
    outlineOff();
    for (int i = 0; i < 18; i++) {
        if (i % 4 != 3) Renderer::drawCircle(-54 + i * 6.3f, 131, 1.6f, Palette::AMBER);
    }

    // Windows with passengers
    outlineOn(1.5f);
    for (int i = 0; i < 6; i++) Renderer::drawRect(-150 + i * 40.0f, 80, 32, 38, Palette::GLASS);
    Renderer::drawRect(98, 26, 26, 92, Palette::GLASS);                                // door
    Renderer::drawPolygon({ {132, 78}, {157, 78}, {157, 122}, {136, 122} }, Palette::GLASS);  // windshield
    outlineOff();
    Renderer::drawLine(111, 26, 111, 118, 1.5f, Palette::OUTLINE);                      // door split
    for (int i = 0; i < 6; i++) {
        float px = -134 + i * 40.0f;
        if (i % 2 == 0) {
            Renderer::drawCircle(px, 98, 7, {0.22f, 0.18f, 0.20f, 0.75f});
            Renderer::drawRect(px - 9, 80, 18, 10, {0.22f, 0.18f, 0.20f, 0.75f});
        }
        Renderer::drawLine(px - 10, 86, px - 3, 112, 2, {1, 1, 1, 0.3f});
    }

    // Big angry bus driver
    float hx = 146, hy = 100;
    Renderer::drawCircle(hx, hy, 9, Palette::SKIN);
    drawCircleSegment(hx, hy, 9.4f, 25, 195, Palette::HAIR);
    drawEye(hx + 4, hy + 1, 2.2f);
    drawAngryBrow(hx + 4, hy + 4.5f, 7);
    Renderer::drawEllipse(hx + 4, hy - 4, 4, 1.5f, Palette::HAIR);
    drawMouth(hx + 4, hy - 6.5f, 4, anim);

    // Bumper, grille, lights
    outlineOn(1.5f);
    Renderer::drawRect(-164, 22, 328, 8, darker(Palette::METAL, 0.6f));
    Renderer::drawRect(148, 40, 12, 12, Palette::HEADLIGHT);
    Renderer::drawRect(-161, 40, 6, 14, Palette::AMBER);
    outlineOff();
    for (int i = 0; i < 4; i++) Renderer::drawLine(140, 34 + i * 6.0f, 158, 34 + i * 6.0f, 1.5f,
                                                   darker(body, 0.6f));

    // Wheel arches
    drawCircleSegment(-100, 25, 30, 0, 180, darker(body, 0.35f));
    drawCircleSegment( 95, 25, 30, 0, 180, darker(body, 0.35f));
    Renderer::popTransform();

    drawWheel(-100, 25, 25, wheel);
    drawWheel(  95, 25, 25, wheel);
    endModel();
}

// =====================================================================
//  PROJECTILES  (x, y) = CENTER. 'time' makes them spin.
// =====================================================================
void Models::drawStone(float x, float y, float time) {
    Renderer::pushTransform();
    Renderer::translate(x, y);
    Renderer::rotate(-time * 500.0f);
    outlineOn(1.5f);
    Renderer::drawPolygon({ {-5, -3}, {-1, -5.5f}, {4.5f, -3.5f}, {5.5f, 1.5f}, {2, 5}, {-4, 4} },
                          Palette::STONE);
    outlineOff();
    Renderer::drawCircle(-1.5f, 1.5f, 1.5f, lighter(Palette::STONE, 0.4f));
    Renderer::drawLine(1, -3, 3, 0, 1.0f, darker(Palette::STONE, 0.6f));
    Renderer::popTransform();
}

void Models::drawTire(float x, float y, float time) {
    drawWheel(x, y, 14, -time * 540.0f);
}

void Models::drawWatermelon(float x, float y, float time) {
    Renderer::pushTransform();
    Renderer::translate(x, y);
    Renderer::rotate(-time * 400.0f);
    outlineOn(1.5f);
    Renderer::drawEllipse(0, 0, 9, 7, Palette::WATERMELON);
    outlineOff();
    Color stripe = darker(Palette::WATERMELON, 0.55f);
    Renderer::drawLine(-6, -3, 6, -3, 1.6f, stripe);
    Renderer::drawLine(-8, 0, 8, 0, 1.6f, stripe);
    Renderer::drawLine(-6, 3, 6, 3, 1.6f, stripe);
    Renderer::drawEllipse(-3, 3, 2.5f, 1.2f, {1, 1, 1, 0.35f});
    Renderer::popTransform();
}
