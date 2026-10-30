// ============================================================================
//  menus.h : main menu, sound settings (bonus), pause menu, win/lose menu
// ============================================================================
#pragma once
#include "common.h"

void updateMenus(float dt);
void drawMenus();                 // draws whichever menu belongs to g_state
void menuMouseClick(int button);
