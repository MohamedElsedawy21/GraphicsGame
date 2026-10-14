#pragma once
#include "game/World.h"

// Every screen the game can be on
enum class GameState {
    MainMenu,
    SoundSettings,
    Cutscene,
    Playing,
    Paused,
    Won,
    Lost
};

// =====================================================================
//  Game  -  the state machine that connects everything.
//
//    MainMenu --Start--> Cutscene --(ends / skip)--> Playing
//       |  ^                                         |   ^
//  Sound|  |Back                                Esc  |   | Continue / Esc
//       v  |                                         v   |
//   SoundSettings                                    Paused
//
//    Playing --all waves beaten--> Won     Playing --enemy reaches base--> Lost
//    Paused / Won / Lost --Restart--> Playing (fresh world)
//    Paused / Won / Lost --Main Menu--> MainMenu
// =====================================================================
class Game {
public:
    void init();
    void handleInput();        // once per drawn frame
    void update(float dt);     // exactly 60 times per second
    void draw();
    bool wantsToQuit() const { return quit; }

private:
    void changeState(GameState next);
    void restart();
    void removeFinishedEntities();
    void checkWinAndLose();
    void drawWorld();
    void handleDebugKeys();

    GameState state = GameState::MainMenu;
    World world;
    bool  frozen = true;       // true = the game world does not move (menus, pause, win, lose)
    bool  quit = false;
    float clock = 0.0f;        // always running: animates menus even when frozen
};
