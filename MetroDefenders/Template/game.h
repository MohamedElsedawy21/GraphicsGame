// ============================================================================
//  game.h : gameplay (lanes, players, enemies, waves, coins, HUD)
// ============================================================================
#pragma once
#include "common.h"

void resetGame();
void updateGame(float dt);
void drawGame(bool showHud);
void gameMouseClick(int button);
void gameKey(int key);
