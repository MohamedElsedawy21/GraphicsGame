
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

        // Move towards the home base
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

// ==================== Wave 1 Settings ====================

namespace {
    int spawnedTuktuks = 0;
    float spawnTimer = 0.0f;

    const int TOTAL_TUKTUKS = 3;
    const float SPAWN_DELAY = 2.5f;

    void spawnTuktuk(World& world) {
        int lane = std::rand() % Layout::LANE_COUNT;

        world.enemies.push_back(
            std::make_unique<TuktukEnemy>(lane)
        );

        spawnedTuktuks++;
    }
}

// ==================== Enemy System ====================

void EnemySystem::reset(World& world) {
    world.currentWave = 1;
    world.allWavesSpawned = false;

    spawnedTuktuks = 0;
    spawnTimer = 0.0f;

    // Spawn the first Tuk-tuk immediately
    spawnTuktuk(world);
}

void EnemySystem::update(World& world, float dt) {

    // 1. Update enemies and check for defeated enemies
    for (auto& enemy : world.enemies) {

        if (enemy->removeMe) {
            continue;
        }

        enemy->update(dt);

        // Check whether the enemy has been defeated
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

    // 2. Spawn the remaining Tuk-tuks
    if (spawnedTuktuks < TOTAL_TUKTUKS) {
        spawnTimer += dt;

        if (spawnTimer >= SPAWN_DELAY) {
            spawnTimer = 0.0f;
            spawnTuktuk(world);
        }
    }

    // 3. Check whether a player collects a coin
    for (CoinPickup& pickup : world.pickups) {

        if (pickup.collected) {
            continue;
        }

        for (const auto& player : world.players) {

            // Players and coins must be in the same lane
            if (player->lane != pickup.lane) {
                continue;
            }

            // Create a small rectangle around the coin
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
