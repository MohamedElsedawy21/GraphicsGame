#pragma once

// =====================================================================
//  The game scene. Two owners, two .cpp files, so nobody edits the same file:
//    Environment_Person2.cpp : home base (metro station) + left street props
//    Environment_Person3.cpp : sky, buildings, road, right street props, sidewalk
//  The Game draws them in this order (back to front):
//    drawSkyAndBuildings -> drawRoad -> drawHomeBase -> drawLeftProps -> drawRightProps
//    -> (characters) -> drawForeground
// =====================================================================
namespace Environment {
    // Person 3
    void drawSkyAndBuildings(float time);
    void drawRoad(float time);
    void drawRightProps(float time);
    void drawForeground(float time);

    // Person 2
    void drawHomeBase(float time);
    void drawLeftProps(float time);
}
