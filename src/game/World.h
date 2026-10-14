#pragma once
#include <vector>
#include <memory>
#include "shared/Entity.h"
#include "shared/Sizes.h"
#include "game/Coins.h"

// The three player cards
enum class PlayerType { None = -1, Melee = 0, Ranged = 1, Defense = 2 };

inline int cardCost(PlayerType type) {
    switch (type) {
        case PlayerType::Melee:   return Costs::MELEE;
        case PlayerType::Ranged:  return Costs::RANGED;
        case PlayerType::Defense: return Costs::DEFENSE;
        default:                  return 0;
    }
}

// A coin lying on the road, dropped by a defeated enemy
struct CoinPickup {
    float x, y;            // center of the coin
    int   lane;
    float age = 0.0f;      // seconds since it dropped (makes it bob and spin)
    bool  collected = false;
};

// =====================================================================
//  World  -  everything that exists during a game, in one place.
//  Restart = world.reset(), so nothing old can leak into a new game.
//
//  Who writes what:
//    coins            everyone (through add / spend)
//    players          Person 2 (adds and updates them)
//    enemies          Person 3 (adds and updates them)
//    pickups          Person 3 (drops and collects), Person 1 draws them
//    selectedCard     Person 1 (HUD) sets it; Person 2 reads it when placing
//    selectedLanes    Person 2
//    currentWave, allWavesSpawned   Person 3; Person 1 shows the wave and checks the win
// =====================================================================
struct World {
    Coins coins;
    std::vector<std::unique_ptr<Entity>> players;
    std::vector<std::unique_ptr<Entity>> enemies;
    std::vector<CoinPickup> pickups;

    PlayerType selectedCard = PlayerType::None;
    bool selectedLanes[Layout::LANE_COUNT] = {};

    int  currentWave = 0;          // 1, 2 or 3 (0 = not started yet)
    bool allWavesSpawned = false;  // true once the LAST enemy of wave 3 has appeared
    float time = 0.0f;             // seconds of gameplay (stops while paused)

    void reset() {
        coins.reset(Costs::START_COINS);
        players.clear();
        enemies.clear();
        pickups.clear();
        selectedCard = PlayerType::None;
        for (bool& lane : selectedLanes) lane = false;
        currentWave = 0;
        allWavesSpawned = false;
        time = 0.0f;
    }
};
