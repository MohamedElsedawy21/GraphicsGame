#pragma once
#include "game/World.h"

// The bar at the top of the screen during gameplay:
// three player cards on the left, the wave in the middle, coins on the right.
namespace Hud {
    void handleInput(World& world);   // once per frame: clicking cards
    void update(World& world);        // after the coins change: unselect a card that became locked
    void draw(const World& world, float time);

    bool isCardUnlocked(const World& world, PlayerType type);
}
