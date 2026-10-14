#pragma once

// =====================================================================
//  OWNER: Person 4  -  the 10-20 second intro cutscene.
//  The Game calls start() once, then update() and draw() every frame.
//  Skipping (Space / Enter / click) is already handled by the Game.
// =====================================================================
namespace Cutscene {
    void start();               // reset the timeline to 0
    bool update(float dt);      // return true when the cutscene has finished
    void draw();
}
