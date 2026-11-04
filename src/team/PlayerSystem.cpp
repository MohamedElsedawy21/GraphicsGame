
#include "team/PlayerSystem.h"
#include "team/Models.h"

#include "engine/Input.h"
#include "engine/Renderer.h"
#include "engine/Audio.h"

#include "shared/Entity.h"
#include "shared/Palette.h"

#include "ui/Hud.h"

#include <GLFW/glfw3.h>
#include <memory>
#include <vector>

namespace {

    constexpr float MELEE_SPEED = 35.0f;
    constexpr float RANGED_SPEED = 25.0f;

    constexpr float MELEE_EXTRA_RANGE = 20.0f;
    constexpr float RANGED_ATTACK_RANGE = 300.0f;

    constexpr float MELEE_ATTACK_COOLDOWN = 0.8f;
    constexpr float RANGED_ATTACK_COOLDOWN = 1.0f;

    constexpr float STONE_SPEED = 450.0f;
    constexpr float STONE_SIZE = 18.0f;

    struct StoneProjectile {
        float x;
        float y;
        float age;
        int lane;
        bool removeMe = false;
    };

    std::vector<StoneProjectile> stones;

    // ---------- Jump and Duck ----------

    void updateJumpDuck(Entity& player, float& jumpTimer, float dt) {

        // Space OR Arrow Up starts a jump.
        if ((Input::keyPressed(GLFW_KEY_SPACE) ||
             Input::keyPressed(GLFW_KEY_UP)) &&
             !player.anim.jumping) {

            player.anim.jumping = true;
            player.anim.ducking = false;
            jumpTimer = 0.0f;
        }

        // Update the jump animation.
        if (player.anim.jumping) {

            jumpTimer += dt;

            float progress = jumpTimer / Sizes::JUMP_TIME;

            if (progress >= 1.0f) {

                player.anim.jumping = false;
                player.anim.jumpOffset = 0.0f;
                jumpTimer = 0.0f;
            }
            else {

                // Smooth jump arc: rises and then lands.
                player.anim.jumpOffset =
                    Sizes::JUMP_HEIGHT *
                    4.0f * progress * (1.0f - progress);
            }
        }

        // Arrow Down ducks while held.
        // Ducking is disabled during a jump.
        player.anim.ducking =
            Input::keyDown(GLFW_KEY_DOWN) &&
            !player.anim.jumping;
    }

    // ---------- Enemy Targeting ----------

    Entity* findEnemyAhead(
        const Entity& player,
        World& world,
        float maxRange
    ) {
        Entity* closest = nullptr;
        float closestDistance = maxRange;

        for (auto& enemy : world.enemies) {

            if (!enemy->isAlive() || enemy->removeMe)
                continue;

            if (enemy->lane != player.lane)
                continue;

            float distance = enemy->x - player.x;

            if (distance >= 0.0f &&
                distance <= closestDistance) {

                closestDistance = distance;
                closest = enemy.get();
            }
        }

        return closest;
    }

    bool targetStillAlive(Entity* target, World& world) {

        if (target == nullptr)
            return false;

        for (auto& enemy : world.enemies) {

            if (enemy.get() == target) {
                return enemy->isAlive() && !enemy->removeMe;
            }
        }

        return false;
    }

    // ---------- Ranged Projectiles ----------

    void updateStones(World& world, float dt) {

        for (StoneProjectile& stone : stones) {

            if (stone.removeMe)
                continue;

            stone.x += STONE_SPEED * dt;
            stone.age += dt;

            Rect stoneHitbox = {
                stone.x - STONE_SIZE / 2.0f,
                stone.y - STONE_SIZE / 2.0f,
                STONE_SIZE,
                STONE_SIZE
            };

            for (auto& enemy : world.enemies) {

                if (!enemy->isAlive() || enemy->removeMe)
                    continue;

                if (enemy->lane != stone.lane)
                    continue;

                if (overlaps(stoneHitbox, enemy->hitbox())) {

                    enemy->takeDamage(Sizes::PLAYER_DAMAGE);

                    Audio::play(Sfx::Hit);

                    stone.removeMe = true;
                    break;
                }
            }

            if (stone.x > Layout::SCREEN_W + 30.0f) {
                stone.removeMe = true;
            }
        }

        for (int i = static_cast<int>(stones.size()) - 1;
             i >= 0; i--) {

            if (stones[i].removeMe) {
                stones.erase(stones.begin() + i);
            }
        }
    }
}

// ==================== Melee Player ====================

class MeleePlayer : public Entity {
public:

    MeleePlayer(int lane)
        : Entity(Layout::HOME_BASE_X, lane,
                 Sizes::PEDESTRIAN, Sizes::MELEE_HP) {}

    void update(float dt) override {

        anim.time += dt;

        updateJumpDuck(*this, jumpTimer, dt);

        if (attackCooldown > 0.0f)
            attackCooldown -= dt;

        if (anim.attack > 0.0f) {

            anim.attack -= dt * 2.0f;

            if (anim.attack < 0.0f)
                anim.attack = 0.0f;
        }
    }

    void move(World& world, float dt) {

        if (!targetStillAlive(currentTarget, world)) {
            currentTarget = nullptr;
        }

        if (currentTarget == nullptr) {

            float nearestDistance = 100000.0f;

            for (auto& enemy : world.enemies) {

                if (!enemy->isAlive() || enemy->removeMe)
                    continue;

                if (enemy->lane != lane)
                    continue;

                float distance = enemy->x - x;

                float range =
                    size.hitW / 2.0f +
                    enemy->size.hitW / 2.0f +
                    MELEE_EXTRA_RANGE;

                if (distance >= 0.0f &&
                    distance <= range &&
                    distance < nearestDistance) {

                    nearestDistance = distance;
                    currentTarget = enemy.get();
                }
            }
        }

        if (currentTarget != nullptr) {

            anim.moving = false;

            if (attackCooldown <= 0.0f) {

                currentTarget->takeDamage(Sizes::PLAYER_DAMAGE);

                attackCooldown = MELEE_ATTACK_COOLDOWN;
                anim.attack = 1.0f;

                Audio::play(Sfx::Attack);
            }
        }
        else {

            x += MELEE_SPEED * dt;
            anim.moving = true;
        }
    }

    void draw() const override {

        Models::drawPedestrian(x, drawY(), 1.0f, anim);
        drawHealth();
    }

private:

    float attackCooldown = 0.0f;
    float jumpTimer = 0.0f;

    Entity* currentTarget = nullptr;
};

// ==================== Ranged Player ====================

class RangedPlayer : public Entity {
public:

    RangedPlayer(int lane)
        : Entity(Layout::HOME_BASE_X, lane,
                 Sizes::STONE_THROWER, Sizes::RANGED_HP) {}

    void update(float dt) override {

        anim.time += dt;

        updateJumpDuck(*this, jumpTimer, dt);

        if (attackCooldown > 0.0f)
            attackCooldown -= dt;

        if (anim.attack > 0.0f) {

            anim.attack -= dt * 2.0f;

            if (anim.attack < 0.0f)
                anim.attack = 0.0f;
        }
    }

    void move(World& world, float dt) {

        if (!targetStillAlive(currentTarget, world)) {
            currentTarget = nullptr;
        }

        if (currentTarget == nullptr) {

            currentTarget = findEnemyAhead(
                *this,
                world,
                RANGED_ATTACK_RANGE
            );
        }

        if (currentTarget != nullptr) {

            anim.moving = false;

            if (attackCooldown <= 0.0f) {

                stones.push_back({
                    x + 30.0f,
                    drawY() + 55.0f,
                    0.0f,
                    lane
                });

                attackCooldown = RANGED_ATTACK_COOLDOWN;
                anim.attack = 1.0f;

                Audio::play(Sfx::Attack);
            }
        }
        else {

            x += RANGED_SPEED * dt;
            anim.moving = true;
        }
    }

    void draw() const override {

        Models::drawStoneThrower(x, drawY(), 1.0f, anim);
        drawHealth();
    }

private:

    float attackCooldown = 0.0f;
    float jumpTimer = 0.0f;

    Entity* currentTarget = nullptr;
};

// ==================== Defense Player ====================

class DefensePlayer : public Entity {
public:

    DefensePlayer(int lane)
        : Entity(Layout::HOME_BASE_X, lane,
                 Sizes::BARRIER, Sizes::DEFENSE_HP) {

        anim.moving = false;
    }

    void update(float dt) override {

        anim.time += dt;
        anim.moving = false;
    }

    void draw() const override {

        Models::drawBarrier(x, drawY(), 1.0f, anim);
        drawHealth();
    }
};

// ==================== Player System ====================

namespace PlayerSystem {

void placePlayers(World& world) {

    PlayerType type = world.selectedCard;

    if (type == PlayerType::None)
        return;

    if (!Hud::isCardUnlocked(world, type))
        return;

    int selectedCount = 0;

    for (int lane = 0; lane < Layout::LANE_COUNT; lane++) {

        if (world.selectedLanes[lane])
            selectedCount++;
    }

    if (selectedCount == 0)
        return;

    int cost = cardCost(type) * selectedCount;

    if (!world.coins.spend(cost))
        return;

    for (int lane = 0; lane < Layout::LANE_COUNT; lane++) {

        if (!world.selectedLanes[lane])
            continue;

        if (type == PlayerType::Melee) {

            world.players.push_back(
                std::make_unique<MeleePlayer>(lane)
            );
        }
        else if (type == PlayerType::Ranged) {

            world.players.push_back(
                std::make_unique<RangedPlayer>(lane)
            );
        }
        else if (type == PlayerType::Defense) {

            world.players.push_back(
                std::make_unique<DefensePlayer>(lane)
            );
        }

        Audio::play(Sfx::Place);

        world.selectedLanes[lane] = false;
    }
}

void reset(World& world) {

    for (int lane = 0; lane < Layout::LANE_COUNT; lane++) {
        world.selectedLanes[lane] = false;
    }

    stones.clear();
}

void handleInput(World& world) {

    if (Input::keyPressed(GLFW_KEY_ENTER)) {

        placePlayers(world);
        return;
    }

    if (!Input::mouseClicked())
        return;

    int lane = Layout::laneAt(Input::mouseY());

    if (lane < 0 || lane >= Layout::LANE_COUNT)
        return;

    world.selectedLanes[lane] =
        !world.selectedLanes[lane];

    Audio::play(Sfx::LaneSelect);
}

void update(World& world, float dt) {

    for (auto& player : world.players) {

        if (!player->isAlive() || player->removeMe)
            continue;

        player->update(dt);

        MeleePlayer* melee =
            dynamic_cast<MeleePlayer*>(player.get());

        if (melee != nullptr) {

            melee->move(world, dt);
            continue;
        }

        RangedPlayer* ranged =
            dynamic_cast<RangedPlayer*>(player.get());

        if (ranged != nullptr) {

            ranged->move(world, dt);
            continue;
        }
    }

    updateStones(world, dt);
}

void drawUnderCharacters(const World& world) {

    int hoveredLane = Layout::laneAt(Input::mouseY());

    for (int lane = 0; lane < Layout::LANE_COUNT; lane++) {

        if (world.selectedLanes[lane]) {

            Renderer::drawRect(
                0,
                Layout::laneBottom(lane),
                Layout::SCREEN_W,
                Layout::LANE_HEIGHT,
                Palette::LANE_SELECT
            );
        }
        else if (lane == hoveredLane) {

            Renderer::drawRect(
                0,
                Layout::laneBottom(lane),
                Layout::SCREEN_W,
                Layout::LANE_HEIGHT,
                Palette::LANE_HOVER
            );
        }
    }
}

void drawOverCharacters(const World&) {

    for (const StoneProjectile& stone : stones) {

        Models::drawStone(
            stone.x,
            stone.y,
            stone.age
        );
    }
}


bool interceptEnemyProjectile(
    World& world,
    int lane,
    const Rect& projectileHitbox,
    int damage
) {
    for (auto& player : world.players) {

        if (!player->isAlive() || player->removeMe)
            continue;

        if (player->lane != lane)
            continue;

        DefensePlayer* defense =
            dynamic_cast<DefensePlayer*>(player.get());

        if (defense == nullptr)
            continue;

        if (overlaps(defense->hitbox(), projectileHitbox)) {

            defense->takeDamage(damage);

            Audio::play(Sfx::Hit);

            return true;
        }
    }

    return false;
}
}
