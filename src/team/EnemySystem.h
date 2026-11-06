
#pragma once

#include "game/World.h"

// Enemy system: spawning, waves, combat and coin drops.
namespace EnemySystem {

    // Reset enemy state and prepare wave 1.
    void reset(World& world);

    // Spawn enemies, update movement, attacks and collect coins.
    void update(World& world, float dt);

    // Draw projectiles and effects above characters.
    void drawOverCharacters(const World& world);

}
