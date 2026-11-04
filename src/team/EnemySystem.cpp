
#include "team/EnemySystem.h"
#include "team/Models.h"
#include "engine/Renderer.h"
#include "engine/Audio.h"

#include <cstdlib>
#include <vector>
#include <cmath>

// ==================== Projectile Structures ====================

struct WatermelonProjectile {
    float x;
    float startY;
    float age;
    int lane;
};

struct TireProjectile {
    float x;
    float y;
    float age;
    int lane;
};

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

    bool tryThrow(float dt, float& projectileX, float& projectileY) {
        attackTimer += dt;

        if (attackTimer >= 3.0f) {
            attackTimer = 0.0f;

            projectileX = x - 55.0f;
            projectileY = drawY() + 65.0f;

            return true;
        }

        return false;
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

private:
    float attackTimer = 0.0f;
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

    bool tryLaunchTire(float dt, float& tireX, float& tireY) {
        attackTimer += dt;

        if (attackTimer >= 3.5f) {
            attackTimer = 0.0f;

            tireX = x - 65.0f;
            tireY = drawY() + 15.0f;

            return true;
        }

        return false;
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

private:
    float attackTimer = 0.0f;
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

    const float WATERMELON_SPEED = 260.0f;
    const float WATERMELON_LIFETIME = 1.2f;
    const float WATERMELON_ARC_HEIGHT = 70.0f;

    const float TIRE_SPEED = 300.0f;
    const float TIRE_LIFETIME = 2.5f;

    int spawnedThisWave = 0;
    float spawnTimer = 0.0f;

    std::vector<WatermelonProjectile> watermelons;
    std::vector<TireProjectile> tires;

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

        if (world.currentWave == 3 &&
            spawnedThisWave == getWaveSize(3)) {
            world.allWavesSpawned = true;
        }
    }

    void startWave(World& world, int wave) {
        world.currentWave = wave;

        spawnedThisWave = 0;
        spawnTimer = 0.0f;

        spawnEnemy(world);
    }
}

// ==================== Enemy System ====================

void EnemySystem::reset(World& world) {
    world.allWavesSpawned = false;

    spawnedThisWave = 0;
    spawnTimer = 0.0f;

    watermelons.clear();
    tires.clear();

    startWave(world, 1);
}

void EnemySystem::update(World& world, float dt) {

    // 1. Update enemies and check for defeated enemies.

    for (auto& enemy : world.enemies) {

        if (enemy->removeMe) {
            continue;
        }

        enemy->update(dt);

        if (!enemy->isAlive()) {
            Audio::play(Sfx::Death);

            world.pickups.push_back({
                enemy->x,
                enemy->y + 20.0f,
                enemy->lane
            });

            enemy->removeMe = true;
            continue;
        }

        // 2. Tuk-tuk watermelon attack.

        TuktukEnemy* tuktuk =
            dynamic_cast<TuktukEnemy*>(enemy.get());

        if (tuktuk != nullptr) {
            float projectileX;
            float projectileY;

            if (tuktuk->tryThrow(dt, projectileX, projectileY)) {

                watermelons.push_back({
                    projectileX,
                    projectileY,
                    0.0f,
                    enemy->lane
                });

                Audio::play(Sfx::Attack);
            }
        }

        // 3. Microbus tire attack.

        MicrobusEnemy* microbus =
            dynamic_cast<MicrobusEnemy*>(enemy.get());

        if (microbus != nullptr) {
            float tireX;
            float tireY;

            if (microbus->tryLaunchTire(dt, tireX, tireY)) {

                tires.push_back({
                    tireX,
                    tireY,
                    0.0f,
                    enemy->lane
                });

                Audio::play(Sfx::Attack);
            }
        }
    }

    // 4. Update watermelon projectiles.

    for (WatermelonProjectile& watermelon : watermelons) {
        watermelon.age += dt;
        watermelon.x -= WATERMELON_SPEED * dt;
    }

    // Remove watermelons after their flight.

    for (int i = static_cast<int>(watermelons.size()) - 1;
         i >= 0; i--) {

        if (watermelons[i].age >= WATERMELON_LIFETIME) {
            watermelons.erase(watermelons.begin() + i);
        }
    }

    // 5. Update rolling tire projectiles.

    for (TireProjectile& tire : tires) {
        tire.age += dt;
        tire.x -= TIRE_SPEED * dt;
    }

    // Remove tires after their travel time.

    for (int i = static_cast<int>(tires.size()) - 1;
         i >= 0; i--) {

        if (tires[i].age >= TIRE_LIFETIME) {
            tires.erase(tires.begin() + i);
        }
    }

    // 6. Spawn the remaining enemies in the current wave.

    int totalEnemies = getWaveSize(world.currentWave);

    if (spawnedThisWave < totalEnemies) {
        spawnTimer += dt;

        if (spawnTimer >= SPAWN_DELAY) {
            spawnTimer = 0.0f;
            spawnEnemy(world);
        }
    }

    // 7. Start the next wave only when all current enemies are defeated.

    if (spawnedThisWave == totalEnemies && world.enemies.empty()) {

        if (world.currentWave < 3) {
            startWave(world, world.currentWave + 1);
        }
    }

    // 8. Check whether a player collects a coin.

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

// ==================== Draw Effects ====================

void EnemySystem::drawOverCharacters(const World& world) {

    // Draw watermelon projectiles with an arc.

    for (const WatermelonProjectile& watermelon : watermelons) {

        float progress = watermelon.age / WATERMELON_LIFETIME;

        float arc = WATERMELON_ARC_HEIGHT *
                    std::sin(progress * 3.14159265f);

        float projectileY = watermelon.startY + arc;

        Models::drawWatermelon(
            watermelon.x,
            projectileY,
            watermelon.age
        );
    }

    // Draw rolling tires.

    for (const TireProjectile& tire : tires) {

        Models::drawTire(
            tire.x,
            tire.y,
            tire.age
        );
    }
}
