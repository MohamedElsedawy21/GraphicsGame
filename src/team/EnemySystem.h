#pragma once
#include "game/World.h"

// =====================================================================
//  OWNER: Person 3  -  enemies, waves, combat, coin drops.
//  The Game calls these at the right moments; fill in the bodies.
// =====================================================================
namespace EnemySystem {
    void reset(World& world);                     // new game: wave 1, empty timers
    void update(World& world, float dt);          // 60 times a second: spawn, move, fight,
                                                  //   drop coins, collect coins
    void drawOverCharacters(const World& world);  // tires and thrown objects
}
