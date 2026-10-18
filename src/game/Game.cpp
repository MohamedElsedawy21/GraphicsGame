#include "game/Game.h"
#include "engine/Renderer.h"
#include "engine/Text.h"
#include "engine/Input.h"
#include "engine/Audio.h"
#include "shared/Palette.h"
#include "ui/Hud.h"
#include "ui/Menus.h"
#include "ui/CoinModel.h"
#include "team/PlayerSystem.h"
#include "team/EnemySystem.h"
#include "team/Environment.h"
#include "team/Cutscene.h"
#define GLFW_INCLUDE_NONE   // we only need GLFW key names here, not OpenGL
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>

// Testing shortcuts while the other parts are not ready yet:
//   F1 = +10 coins   F2 = win now   F3 = lose now   F4 = drop a coin
// NOTE: Set this to false before recording the video and submitting!
static const bool DEBUG_KEYS = true;

// ===== Section A: setup and state changes =====
void Game::init() {
    world.reset();
    changeState(GameState::MainMenu);
    Audio::startMusic();
}

void Game::changeState(GameState next) {
    state = next;
    // The world only moves while playing; everything else freezes it
    frozen = (state != GameState::Playing);

    if (state == GameState::Cutscene) Cutscene::start();
    Audio::setMusicVolume(state == GameState::Cutscene ? 0.3f : 1.0f);   // let voices be heard
    if (state == GameState::Won)  { Audio::play(Sfx::Win); }
    if (state == GameState::Lost) { Audio::stopMusic(); Audio::play(Sfx::Lose); }
    if (state == GameState::MainMenu || state == GameState::Playing) Audio::startMusic();
}

void Game::restart() {
    world.reset();
    PlayerSystem::reset(world);
    EnemySystem::reset(world);
    changeState(GameState::Playing);
}

// ===== Section B: input (once per frame) =====
void Game::handleInput() {
    switch (state) {
        case GameState::MainMenu: {
            MenuAction a = Menus::handleMain();
            if (a == MenuAction::Start)     changeState(GameState::Cutscene);
            if (a == MenuAction::OpenSound) changeState(GameState::SoundSettings);
            if (a == MenuAction::Exit)      quit = true;
            break;
        }
        case GameState::SoundSettings: {
            MenuAction a = Menus::handleSound();
            if (a == MenuAction::VolumeUp)   { Audio::volumeUp();   Audio::play(Sfx::Coin); }
            if (a == MenuAction::VolumeDown) { Audio::volumeDown(); Audio::play(Sfx::Coin); }
            if (a == MenuAction::ToggleMute)   Audio::toggleMute();
            if (a == MenuAction::Back || Input::keyPressed(GLFW_KEY_ESCAPE))
                changeState(GameState::MainMenu);
            break;
        }
        case GameState::Cutscene: {
            if (Input::keyPressed(GLFW_KEY_SPACE) || Input::keyPressed(GLFW_KEY_ENTER) ||
                Input::mouseClicked()) {
                restart();   // skip straight to a fresh game
            }
            break;
        }
        case GameState::Playing: {
            if (Input::keyPressed(GLFW_KEY_ESCAPE)) { changeState(GameState::Paused); break; }
            Hud::handleInput(world);
            PlayerSystem::handleInput(world);
            if (DEBUG_KEYS) handleDebugKeys();
            break;
        }
        case GameState::Paused: {
            MenuAction a = Menus::handlePause();
            if (a == MenuAction::Continue || Input::keyPressed(GLFW_KEY_ESCAPE))
                changeState(GameState::Playing);
            if (a == MenuAction::Restart)  restart();
            if (a == MenuAction::MainMenu) changeState(GameState::MainMenu);
            break;
        }
        case GameState::Won:
        case GameState::Lost: {
            MenuAction a = Menus::handleEnd();
            if (a == MenuAction::Restart)  restart();
            if (a == MenuAction::MainMenu) changeState(GameState::MainMenu);
            break;
        }
    }
}

void Game::handleDebugKeys() {
    if (Input::keyPressed(GLFW_KEY_F1)) world.coins.add(10);
    if (Input::keyPressed(GLFW_KEY_F2)) changeState(GameState::Won);
    if (Input::keyPressed(GLFW_KEY_F3)) changeState(GameState::Lost);
    if (Input::keyPressed(GLFW_KEY_F4)) {
        int lane = std::rand() % Layout::LANE_COUNT;
        float x = 300.0f + std::rand() % 900;
        world.pickups.push_back({ x, Layout::laneGroundY(lane) + 20.0f, lane });
    }
}

// ===== Section C: update (60 times per second) =====
void Game::update(float dt) {
    clock += dt;

    if (state == GameState::Cutscene) {
        if (Cutscene::update(dt)) restart();   // cutscene finished -> start playing
        return;
    }

    if (frozen) return;   // pause, win and lose: nothing in the world moves

    world.time += dt;
    PlayerSystem::update(world, dt);
    EnemySystem::update(world, dt);
    for (CoinPickup& p : world.pickups) p.age += dt;

    removeFinishedEntities();
    Hud::update(world);
    checkWinAndLose();
}

void Game::removeFinishedEntities() {
    auto finished = [](const std::unique_ptr<Entity>& e) { return e->removeMe; };
    world.players.erase(std::remove_if(world.players.begin(), world.players.end(), finished),
                        world.players.end());
    world.enemies.erase(std::remove_if(world.enemies.begin(), world.enemies.end(), finished),
                        world.enemies.end());

    auto collected = [](const CoinPickup& p) { return p.collected; };
    world.pickups.erase(std::remove_if(world.pickups.begin(), world.pickups.end(), collected),
                        world.pickups.end());
}

void Game::checkWinAndLose() {
    // LOSE: any living enemy touches the home base
    for (const auto& enemy : world.enemies) {
        if (enemy->isAlive() && enemy->hitbox().x <= Layout::HOME_BASE_X) {
            changeState(GameState::Lost);
            return;
        }
    }
    // WIN: every wave has finished spawning and no enemies are left
    if (world.allWavesSpawned && world.enemies.empty()) {
        changeState(GameState::Won);
    }
}

// ===== Section D: drawing =====
void Game::drawWorld() {
    float t = world.time;

    // Back to front, like painting a picture
    Environment::drawSkyAndBuildings(t);
    Environment::drawRoad(t);
    Environment::drawHomeBase(t);
    Environment::drawLeftProps(t);
    Environment::drawRightProps(t);
    PlayerSystem::drawUnderCharacters(world);

    // Characters lane by lane, top lane first, so lower lanes overlap the ones behind
    for (int lane = 0; lane < Layout::LANE_COUNT; lane++) {
        for (const auto& p : world.players) if (p->lane == lane) p->draw();
        for (const auto& e : world.enemies) if (e->lane == lane) e->draw();
    }

    // Coins on the road bob up and down and spin
    for (const CoinPickup& p : world.pickups) {
        float bob = std::sin(p.age * 4.0f) * 4.0f;
        Renderer::drawEllipse(p.x, p.y - 16, 10, 3, Palette::SHADOW);
        drawCoin(p.x, p.y + bob, 12, p.age + 0.01f);
    }

    EnemySystem::drawOverCharacters(world);
    PlayerSystem::drawOverCharacters(world);
    Environment::drawForeground(t);

    Hud::draw(world, clock);
}

void Game::draw() {
    switch (state) {
        case GameState::MainMenu:      Menus::drawMain(clock);  break;
        case GameState::SoundSettings: Menus::drawSound(clock); break;
        case GameState::Cutscene:
            Cutscene::draw();
            Text::drawShadowed("PRESS SPACE TO SKIP", 1040, 20, 12, Palette::UI_TEXT, Palette::OUTLINE);
            break;
        case GameState::Playing: drawWorld(); break;
        case GameState::Paused:  drawWorld(); Menus::drawPause(); break;
        case GameState::Won:     drawWorld(); Menus::drawEnd(true, clock);  break;
        case GameState::Lost:    drawWorld(); Menus::drawEnd(false, clock); break;
    }
}
