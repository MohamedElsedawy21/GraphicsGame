#pragma once
#include "shared/AnimState.h"

// =====================================================================
//  OWNER: Person 4  -  the six character models (+ projectiles).
//  Every function: (x, y) = BOTTOM-CENTER of the model, facing RIGHT.
//  scale 1.0 = the size from Sizes.h. The HUD cards call these at scale 0.5.
//  Persons 2 and 3 call them from their entities' draw() functions.
//  To face left (enemies): Renderer::pushTransform(); translate(x, y);
//  scale(-1, 1); drawCar(0, 0, 1, anim); popTransform();
// =====================================================================
namespace Models {
    // Players
    void drawPedestrian(float x, float y, float scale, const AnimState& anim);    // Melee
    void drawStoneThrower(float x, float y, float scale, const AnimState& anim);  // Ranged
    void drawBarrier(float x, float y, float scale, const AnimState& anim);       // Defense

    // Enemies
    void drawTuktuk(float x, float y, float scale, const AnimState& anim);        // Easy
    void drawMicrobus(float x, float y, float scale, const AnimState& anim);      // Medium
    void drawCar(float x, float y, float scale, const AnimState& anim);           // Hard

    // Projectiles: (x, y) = CENTER
    void drawStone(float x, float y, float time);
    void drawTire(float x, float y, float time);
    void drawWatermelon(float x, float y, float time);
}
