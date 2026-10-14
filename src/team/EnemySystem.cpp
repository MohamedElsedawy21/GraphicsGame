// =====================================================================
//  OWNER: Person 3.  EMPTY STUBS - replace with the real logic.
//
//  Your jobs that talk to Person 1's code:
//    - set world.currentWave (1, 2, 3) when each wave begins (the HUD shows it)
//    - set world.allWavesSpawned = true when the LAST enemy of wave 3 appears
//      (the Game then declares a win once world.enemies is empty)
//    - when an enemy dies: Audio::play(Sfx::Death);
//        world.pickups.push_back({ enemy->x, enemy->y + 20, enemy->lane });
//        enemy->removeMe = true;
//    - when a player touches a pickup: pickup.collected = true;
//        world.coins.add(Costs::COIN_VALUE); Audio::play(Sfx::Coin);
//    - the Game checks the LOSE condition by itself (enemy hitbox reaches
//      Layout::HOME_BASE_X), so you don't need to.
// =====================================================================
#include "team/EnemySystem.h"

void EnemySystem::reset(World&) {}
void EnemySystem::update(World&, float) {}
void EnemySystem::drawOverCharacters(const World&) {}
