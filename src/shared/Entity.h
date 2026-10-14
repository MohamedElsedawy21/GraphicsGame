#pragma once
#include "shared/Sizes.h"
#include "shared/AnimState.h"

// A rectangle. (x, y) is the BOTTOM-LEFT corner.
struct Rect {
    float x, y, w, h;
};

// True if two rectangles touch or overlap
bool overlaps(const Rect& a, const Rect& b);

// =====================================================================
//  Entity  -  the base class for every player and every enemy.
//  Persons 2 and 3 make their own classes from it, for example:
//
//      class MeleePlayer : public Entity {
//      public:
//          MeleePlayer(int lane)
//              : Entity(Layout::HOME_BASE_X, lane, Sizes::PEDESTRIAN, Sizes::MELEE_HP) {}
//          void update(float dt) override { ... walk, fight ... }
//          void draw() const override { Models::drawPedestrian(x, drawY(), 1.0f, anim);
//                                       drawHealth(); }
//      };
//
//  Removal: when a character dies, its owner does whatever is needed
//  (death sound, drop a coin) and then sets  removeMe = true.
//  The Game deletes every entity with removeMe == true at the end of the update.
// =====================================================================
class Entity {
public:
    Entity(float x, int lane, const CharSize& size, int maxHp);
    virtual ~Entity() = default;

    virtual void update(float dt) = 0;
    virtual void draw() const = 0;

    Rect  hitbox() const;          // centered on x, standing on y (includes jump height)
    void  takeDamage(int amount);  // lowers hp, never below 0, updates anim.damage
    bool  isAlive() const { return hp > 0; }
    float damageAmount() const;    // 0 (full health) -> 1 (no health): use with setDamage
    float drawY() const { return y + anim.jumpOffset; }
    void  drawHealth() const;      // health bar just above the character's box

    float    x;      // bottom-center x, in pixels
    float    y;      // ground y of the lane
    int      lane;   // 0 = top lane ... 3 = bottom lane
    CharSize size;
    int      hp;
    int      maxHp;
    AnimState anim;
    bool     removeMe = false;
};
