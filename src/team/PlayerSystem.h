
#pragma once

#include "game/World.h"

namespace PlayerSystem {

    void reset(World& world);
    void handleInput(World& world);
    void update(World& world, float dt);
    void drawUnderCharacters(const World& world);
    void drawOverCharacters(const World& world);

    bool interceptEnemyProjectile(
        World& world,
        int lane,
        const Rect& projectileHitbox,
        int damage
    );
}
