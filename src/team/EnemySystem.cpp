
#include "team/EnemySystem.h"
#include "team/Models.h"
#include "engine/Renderer.h"
#include "engine/Audio.h"

#include <cstdlib>

// ==================== Tuk-tuk Enemy ====================

class TuktukEnemy : public Entity {
public:
    TuktukEnemy(int lane)
        : Entity(Layout::SPAWN_X, lane, Sizes::TUKTUK, Sizes::EASY_HP) {
    }

    void update(float dt) override {
        anim.time += dt;
        x -= 40.0f * dt;
    }

    void draw() const override {
        Renderer::pushTransform();
        Renderer::translate(x, drawY());
        Renderer::scale(-1.0f, 1.0f);

        Renderer::setDamage(anim.damage);
        Models::drawTuktuk(0, 0, 1.0f, anim);
        Renderer::setDamage(0.0f);

        Renderer::popTransform();
        drawHealth();
    }
};

// ==================== Microbus Enemy ====================

class MicrobusEnemy : public Entity {
public:
    MicrobusEnemy(int lane)
        : Entity(Layout::SPAWN_X, lane, Sizes::MICROBUS, Sizes::MEDIUM_HP) {
    }

    void update(float dt) override {
        anim.time += dt;
        x -= 45.0f * dt;
    }

    void draw() const override {
        Renderer::pushTransform();
        Renderer::translate(x, drawY());
        Renderer::scale(-1.0f, 1.0f);

        Renderer::setDamage(anim.damage);
        Models::drawMicrobus(0, 0, 1.0f, anim);
        Renderer::setDamage(0.0f);

        Renderer::popTransform();
        drawHealth();
    }
};

// ==================== Car Enemy ====================

class CarEnemy : public Entity {
public:
    CarEnemy(int lane)
        : Entity(Layout::SPAWN_X, lane, Sizes::CAR, Sizes::HARD_HP) {
    }

    void update(float dt) override {
        anim.time += dt;
        x -= 60.0f * dt;
    }

    void draw() const override {
        Renderer::pushTransform();
        Renderer::translate(x, drawY());
        Renderer::scale(-1.0f, 1.0f);

        Renderer::setDamage(anim.damage);
        Models::drawCar(0, 0, 1.0f, anim);
        Renderer::setDamage(0.0f);

        Renderer::popTransform();
        drawHealth();
    }
};

// ==================== Wave Settings ====================

namespace {
    const float SPAWN_DELAY = 2.5f;

    int spawnedThisWave = 0;
    float spawnTimer = 0.0f;

    // Enemy types:
    // 0 = Tuk-tuk
    // 1 = Microbus
    // 2 = Car

    const int WAVE_1[] = {
        0, 0, 0
    };

    const int WAVE_2[] = {
        0, 1, 0, 1, 0
    };

    const int WAVE_3[] = {
        0, 1, 2, 0, 1, 2, 0, 1, 2, 0
    };

    int getWaveSize(int wave) {
        switch (wave) {
            case 1: return 3;
            case 2: return 5;
            case 3: return 10;
            default: return 0;
        }
    }

    int getEnemyType(int wave, int index) {
        switch (wave) {
            case 1: return WAVE_1[index];
            case 2: return WAVE_2[index];
            case 3: return WAVE_3[index];
            default: return 0;
        }
    }

    void spawnEnemy(World& world) {
        int lane = std::rand() % Layout::LANE_COUNT;
        int type = getEnemyType(world.currentWave, spawnedThisWave);

        if (type == 0) {
            world.enemies.push_back(
                std::make_unique<TuktukEnemy>(lane)
            );
        }
        else if (type == 1) {
            world.enemies.push_back(
                std::make_unique<MicrobusEnemy>(lane)
            );
        }
        else if (type == 2) {
            world.enemies.push_back(
                std::make_unique<CarEnemy>(lane)
            );
        }

        spawnedThisWave++;

        // The last enemy of Wave 3 has appeared
        if (world.currentWave == 3 &&
            spawnedThisWave == getWaveSize(3)) {
            world.allWavesSpawned = true;
        }
    }

    void startWave(World& world, int wave) {
        world.currentWave = wave;

        spawnedThisWave = 0;
        spawnTimer = 0.0f;

        // Spawn the first enemy immediately
        spawnEnemy(world);
    }
}

// ==================== Enemy System ====================

void EnemySystem::reset(World& world) {
    world.allWavesSpawned = false;

    spawnedThisWave = 0;
    spawnTimer = 0.0f;

    // Start Wave 1
    startWave(world, 1);
}

void EnemySystem::update(World& world, float dt) {

    // 1. Update enemies and check for defeated enemies
    for (auto& enemy : world.enemies) {

        if (enemy->removeMe) {
            continue;
        }

        enemy->update(dt);

        if (!enemy->isAlive()) {
            Audio::play(Sfx::Death);

            // Drop a coin where the enemy died
            world.pickups.push_back({
                enemy->x,
                enemy->y + 20.0f,
                enemy->lane
            });

            enemy->removeMe = true;
        }
    }

    // 2. Spawn the remaining enemies in the current wave
    int totalEnemies = getWaveSize(world.currentWave);

    if (spawnedThisWave < totalEnemies) {
        spawnTimer += dt;

        if (spawnTimer >= SPAWN_DELAY) {
            spawnTimer = 0.0f;
            spawnEnemy(world);
        }
    }

    // 3. Start the next wave only when all current enemies are defeated
    if (spawnedThisWave == totalEnemies && world.enemies.empty()) {

        if (world.currentWave < 3) {
            startWave(world, world.currentWave + 1);
        }
    }

    // 4. Check whether a player collects a coin
    for (CoinPickup& pickup : world.pickups) {

        if (pickup.collected) {
            continue;
        }

        for (const auto& player : world.players) {

            if (player->lane != pickup.lane) {
                continue;
            }

            Rect coinHitbox = {
                pickup.x - 12.0f,
                pickup.y - 12.0f,
                24.0f,
                24.0f
            };

            if (overlaps(player->hitbox(), coinHitbox)) {
                pickup.collected = true;

                world.coins.add(Costs::COIN_VALUE);
                Audio::play(Sfx::Coin);

                break;
            }
        }
    }
}

void EnemySystem::drawOverCharacters(const World& world) {
}
