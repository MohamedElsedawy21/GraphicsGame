#pragma once
#include "game/World.h"

// =====================================================================
//  OWNER: Person 2  -  lanes, placing players, player behavior.
//  The Game calls these at the right moments; fill in the bodies.
// =====================================================================
namespace PlayerSystem {
    void reset(World& world);                     // new game: clear your own extra state
    void handleInput(World& world);               // once per frame: lane hover/click,
                                                  //   placing players, W/S jump & duck
    void update(World& world, float dt);          // 60 times a second: move, fight, die
    void drawUnderCharacters(const World& world); // lane highlights
    void drawOverCharacters(const World& world);  // stones in the air
}
