#pragma once

// =====================================================================
//  Sizes.h  -  every shared number: character sizes, stats, costs, layout.
//  Change a number HERE and the whole game follows.
// =====================================================================

// Box = the full drawing size. Hitbox = the smaller rectangle used for collisions.
// All characters: origin at BOTTOM-CENTER, facing RIGHT.
struct CharSize {
    float w, h;         // box
    float hitW, hitH;   // hitbox
};

namespace Sizes {
    // ---- Characters (from the model spec) ----
    inline constexpr CharSize PEDESTRIAN    { 60, 100,  40, 90 };   // Melee player
    inline constexpr CharSize STONE_THROWER { 60, 100,  40, 90 };   // Ranged player
    inline constexpr CharSize BARRIER       { 70,  80,  60, 75 };   // Defense player
    inline constexpr CharSize CAR           {150,  80, 130, 65 };   // Hard enemy
    inline constexpr CharSize TUKTUK        {110, 105,  90, 90 };   // Easy enemy
    inline constexpr CharSize MICROBUS      {170, 110, 150, 100};   // Medium enemy
    inline constexpr CharSize BUS           {320, 150,   0,   0};   // cutscene only

    // ---- Health (from the project description) ----
    inline constexpr int MELEE_HP   = 5;
    inline constexpr int RANGED_HP  = 5;
    inline constexpr int DEFENSE_HP = 7;
    inline constexpr int EASY_HP    = 5;
    inline constexpr int MEDIUM_HP  = 5;
    inline constexpr int HARD_HP    = 7;

    // ---- Damage per attack ----
    inline constexpr int PLAYER_DAMAGE = 1;
    inline constexpr int ENEMY_DAMAGE  = 1;
    inline constexpr int HARD_DAMAGE   = 2;

    // ---- Jump and duck (Person 2) ----
    inline constexpr float JUMP_HEIGHT    = 60.0f;
    inline constexpr float JUMP_TIME      = 0.6f;
    inline constexpr float DUCK_HITBOX_H  = 50.0f;
}

// =====================================================================
//  Coins and costs.
//  NOTE: Still to confirm with the TA: the spec asks for Ranged = 2x and
//  Defense = 3x the Melee cost, AND only Melee unlocked at the start
//  with enough coins for 3 Melee. Those rules conflict, so all the
//  numbers live here and can be changed in seconds.
// =====================================================================
namespace Costs {
    inline constexpr int MELEE       = 10;
    inline constexpr int RANGED      = MELEE * 2;   // 2x Melee
    inline constexpr int DEFENSE     = MELEE * 3;   // 3x Melee
    inline constexpr int START_COINS = 30;          // buys 3 Melee
    inline constexpr int COIN_VALUE  = 10;          // one dropped coin is worth this much
}

// =====================================================================
//  Screen layout (1280 x 720, y = 0 at the BOTTOM).
//
//   720 +------------------------------+
//       |  HUD (cards, coins)          |  100 px
//   620 +------------------------------+
//       |  lane 0                      |  130 px
//   490 +------------------------------+
//       |  lane 1                      |
//   360 +------------------------------+
//       |  lane 2                      |
//   230 +------------------------------+
//       |  lane 3                      |
//   100 +------------------------------+
//       |  sidewalk / foreground       |  100 px
//     0 +------------------------------+
//   x: 0..160 = metro station (home base), enemies enter at x = 1280
// =====================================================================
namespace Layout {
    inline constexpr float SCREEN_W     = 1280.0f;
    inline constexpr float SCREEN_H     = 720.0f;

    inline constexpr float HUD_HEIGHT   = 100.0f;
    inline constexpr float HUD_Y        = SCREEN_H - HUD_HEIGHT;   // 620: bottom of the HUD

    inline constexpr int   LANE_COUNT   = 4;
    inline constexpr float LANE_HEIGHT  = 130.0f;
    inline constexpr float LANES_TOP    = HUD_Y;                                   // 620
    inline constexpr float LANES_BOTTOM = LANES_TOP - LANE_COUNT * LANE_HEIGHT;    // 100

    inline constexpr float HOME_BASE_X  = 160.0f;   // enemies touching this x = game over
    inline constexpr float SPAWN_X      = SCREEN_W; // enemies appear at the right edge
    inline constexpr float GROUND_OFFSET= 12.0f;    // characters stand this far above the lane bottom

    // Lane 0 is the TOP lane, lane 3 the bottom lane
    inline constexpr float laneBottom(int lane) { return LANES_TOP - (lane + 1) * LANE_HEIGHT; }
    inline constexpr float laneTop(int lane)    { return LANES_TOP - lane * LANE_HEIGHT; }
    inline constexpr float laneGroundY(int lane){ return laneBottom(lane) + GROUND_OFFSET; }

    // Which lane is at this y? Returns -1 if y is outside all lanes.
    inline constexpr int laneAt(float y) {
        if (y < LANES_BOTTOM || y >= LANES_TOP) return -1;
        return (int)((LANES_TOP - y) / LANE_HEIGHT);
    }
}
