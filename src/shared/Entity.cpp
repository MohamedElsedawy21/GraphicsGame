#include "shared/Entity.h"
#include "ui/HealthBar.h"

bool overlaps(const Rect& a, const Rect& b) {
    return a.x <= b.x + b.w && b.x <= a.x + a.w &&
           a.y <= b.y + b.h && b.y <= a.y + a.h;
}

Entity::Entity(float x, int lane, const CharSize& size, int maxHp)
    : x(x), y(Layout::laneGroundY(lane)), lane(lane), size(size), hp(maxHp), maxHp(maxHp) {}

Rect Entity::hitbox() const {
    float h = anim.ducking ? Sizes::DUCK_HITBOX_H : size.hitH;
    return { x - size.hitW / 2.0f, drawY(), size.hitW, h };
}

void Entity::takeDamage(int amount) {
    hp -= amount;
    if (hp < 0) hp = 0;
    anim.damage = damageAmount();
}

float Entity::damageAmount() const {
    if (maxHp <= 0) return 0.0f;
    // Grows with every hit; capped at 0.8 so the model never becomes a solid red blob
    return 0.8f * (1.0f - (float)hp / (float)maxHp);
}

void Entity::drawHealth() const {
    drawHealthBar(x, drawY() + size.h + 8.0f, hp, maxHp);
}
