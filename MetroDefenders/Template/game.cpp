// ============================================================================
//  game.cpp : all gameplay logic
// ============================================================================
#include "game.h"
#include "models.h"
#include "audio.h"

// ------------------------------------------------------------------ types
enum PType { P_MELEE, P_RANGED, P_DEFENSE };
enum EType { E_TUKTUK, E_MICROBUS, E_CAR };
enum ProjKind { PROJ_PEBBLE, PROJ_NOTE_HIGH, PROJ_ROCK_LOW };

struct Player {
    PType type;
    int   lane;
    float x;
    int   hp, maxHp;
    Pose  pose;
    float jumpT = -1.0f;      // >= 0 while jumping
    float duckT = -1.0f;      // >= 0 while ducking
    float hitFlash = 0.0f;
    bool  fighting = false;
};

struct Enemy {
    EType type;
    int   lane;
    float x;
    int   hp, maxHp;
    Pose  pose;
    float hitFlash = 0.0f;
    bool  fighting = false;
};

struct Projectile {
    ProjKind kind;
    int   lane;
    float x, y;               // y = height above the lane ground
    float vx;
    float rot = 0.0f;
    float t = 0.0f;
    float startX = 0.0f;      // where it was fired from
};

struct Effect { int kind; int lane; float x, y, t, life; };   // 0 puff, 1 shatter
struct Coin   { int lane; float x; int value; float t; };

// ------------------------------------------------------------------ tuning
// Melee is 10 coins and is the only card unlocked at the start (30 coins = 3 melee).
// The Melee unlock requirement is 20 collectibles, so (as the table says):
//   Ranged = 2 x 20 = 40,  Defense = 3 x 20 = 60
const int   COST[3]       = { 10, 40, 60 };
const char* CARD_NAME[3]  = { "VENDOR", "KID", "OFFICER" };
const int   START_COINS   = 30;
const int   PLAYER_HP[3]  = { 5, 5, 7 };
const int   ENEMY_HP[3]   = { 5, 5, 7 };
const int   ENEMY_DMG[3]  = { 1, 1, 2 };
const int   ENEMY_DROP[3] = { 10, 15, 20 };

const float PLAYER_SPEED = 34.0f;
const float ENEMY_SPEED[3] = { 26.0f, 26.0f, 85.0f };       // the taxi RUNS toward the player
const float P_ATK_PERIOD[3] = { 0.7f, 1.3f, 3.0f };
const float E_ATK_PERIOD[3] = { 2.6f, 2.8f, 1.0f };

// collision extents (world pixels, already scaled): front / back of each body
const float P_FRONT[3] = { 30.0f, 22.0f, 54.0f };
const float P_BACK[3]  = { 16.0f, 12.0f, 14.0f };
const float E_FRONT[3] = { 36.0f, 72.0f, 70.0f };
const float E_BACK[3]  = { 48.0f, 72.0f, 62.0f };

const float JUMP_TIME = 0.75f, JUMP_HEIGHT = 72.0f;
const float DUCK_TIME = 0.85f;

// waves: (count, spawn interval)  - wave i begins with its first spawn, ends with its last
const int   WAVE_COUNT[3]    = { 3, 5, 10 };
const float WAVE_INTERVAL[3] = { 7.0f, 6.0f, 4.2f };

// ------------------------------------------------------------------ state
static std::vector<Player>     players;
static std::vector<Enemy>      enemies;
static std::vector<Projectile> projectiles;
static std::vector<Effect>     effects;
static std::vector<Coin>       coins;

static int   g_coins = START_COINS;
static bool  laneSelected[NUM_LANES];
static int   hoverLane = -1;
static int   hoverCard = -1;
static int   armedCard = -1;

static int   waveIdx = 0;
static std::vector<EType> waveQueue;
static float spawnTimer = 0.0f;
static float breakTimer = 0.0f;       // pause before a wave starts (banner)
static bool  waveRunning = false;
static float gameTime = 0.0f;

// card layout
const float CARD_W = 96, CARD_H = 84, CARD_Y = 670;
static float cardX(int i) { return 190.0f + i * 106.0f; }

// ------------------------------------------------------------------ helpers
static bool cardUnlocked(int i) { return g_coins >= COST[i]; }

static void buildWave(int w)
{
    waveQueue.clear();
    if (w == 0) {
        for (int i = 0; i < 3; ++i) waveQueue.push_back(E_TUKTUK);
    }
    else if (w == 1) {
        for (int i = 0; i < 2; ++i) waveQueue.push_back(E_TUKTUK);
        for (int i = 0; i < 3; ++i) waveQueue.push_back(E_MICROBUS);
    }
    else {
        for (int i = 0; i < 3; ++i) waveQueue.push_back(E_TUKTUK);
        for (int i = 0; i < 4; ++i) waveQueue.push_back(E_MICROBUS);
        for (int i = 0; i < 3; ++i) waveQueue.push_back(E_CAR);
    }
    // shuffle so the mix is random every game
    for (int i = (int)waveQueue.size() - 1; i > 0; --i) {
        int j = rand() % (i + 1);
        EType tmp = waveQueue[i]; waveQueue[i] = waveQueue[j]; waveQueue[j] = tmp;
    }
}

void resetGame()
{
    players.clear(); enemies.clear(); projectiles.clear(); effects.clear(); coins.clear();
    g_coins = START_COINS;
    for (int i = 0; i < NUM_LANES; ++i) laneSelected[i] = false;
    hoverLane = hoverCard = armedCard = -1;
    waveIdx = 0;
    buildWave(0);
    waveRunning = false;
    breakTimer = 2.5f;
    spawnTimer = 0.0f;
    gameTime = 0.0f;
}

static void placePlayer(int type, int lane)
{
    if (g_coins < COST[type]) return;
    g_coins -= COST[type];
    Player p;
    p.type = (PType)type;
    p.lane = lane;
    p.x = PF_LEFT + P_BACK[type] + 4.0f;     // leftmost position of the lane
    p.hp = p.maxHp = PLAYER_HP[type];
    p.pose.t = (float)(rand() % 100) / 37.0f;
    players.push_back(p);
    playSfx("place");
}

static void spawnEnemy(EType type)
{
    Enemy e;
    e.type = type;
    // random lane (never changes); prefer lanes whose entrance is free so vehicles don't spawn on top of each other
    std::vector<int> freeLanes;
    for (int l = 0; l < NUM_LANES; ++l) {
        bool busy = false;
        for (const Enemy& o : enemies)
            if (o.lane == l && o.x - E_FRONT[o.type] > PF_RIGHT - 2.0f * E_BACK[type] - 20.0f) busy = true;
        if (!busy) freeLanes.push_back(l);
    }
    e.lane = freeLanes.empty() ? rand() % NUM_LANES : freeLanes[rand() % freeLanes.size()];
    e.x = PF_RIGHT - E_BACK[type];
    e.hp = e.maxHp = ENEMY_HP[type];
    enemies.push_back(e);
    if (type == E_CAR) playSfx("honk");
}

static bool overlaps(const Player& p, const Enemy& e)
{
    if (p.lane != e.lane) return false;
    float pl = p.x - P_BACK[p.type], pr = p.x + P_FRONT[p.type];
    float el = e.x - E_FRONT[e.type], er = e.x + E_BACK[e.type];
    return pr >= el && pl <= er;
}

static float jumpHeight(const Player& p)
{
    if (p.jumpT < 0) return 0.0f;
    return JUMP_HEIGHT * sinf(PI * p.jumpT / JUMP_TIME);
}

static void addEffect(int kind, int lane, float x, float y, float life)
{
    Effect f; f.kind = kind; f.lane = lane; f.x = x; f.y = y; f.t = 0; f.life = life;
    effects.push_back(f);
}

static void damagePlayer(Player& p, int dmg)
{
    // the Defense player's durability is counted in HITS (7 hits), so every attack
    // it absorbs removes exactly one hit, even the taxi's 2-damage ram
    if (p.type == P_DEFENSE) dmg = 1;
    p.hp -= dmg;
    p.hitFlash = 1.0f;
    if (p.type == P_DEFENSE) p.pose.shake = 4.0f;
    playSfx("hit");
}

// ------------------------------------------------------------------ input
void gameKey(int key)
{
    bool jump = (key == GLFW_KEY_W || key == GLFW_KEY_UP);
    bool duck = (key == GLFW_KEY_S || key == GLFW_KEY_DOWN);
    if (!jump && !duck) return;
    bool any = false;
    for (Player& p : players) {
        if (!laneSelected[p.lane] || p.type == P_DEFENSE) continue;   // defense not affected
        if (p.jumpT >= 0 || p.duckT >= 0) continue;
        if (jump) p.jumpT = 0.0f; else p.duckT = 0.0f;
        any = true;
    }
    if (any) playSfx(jump ? "jump" : "duck");
}

void gameMouseClick(int button)
{
    if (button == GLFW_MOUSE_BUTTON_RIGHT) { armedCard = -1; return; }
    if (button != GLFW_MOUSE_BUTTON_LEFT) return;

    // player cards (locked cards ignore the mouse completely)
    for (int i = 0; i < 3; ++i) {
        if (!pointInRect(g_mouseX, g_mouseY, cardX(i), CARD_Y, CARD_W, CARD_H)) continue;
        if (!cardUnlocked(i)) return;
        bool anySelected = false;
        for (int l = 0; l < NUM_LANES; ++l) if (laneSelected[l]) anySelected = true;
        if (anySelected) {
            for (int l = 0; l < NUM_LANES; ++l)
                if (laneSelected[l]) placePlayer(i, l);
            armedCard = -1;
        }
        else {
            armedCard = (armedCard == i) ? -1 : i;
            playSfx("select");
        }
        return;
    }

    // lanes: click toggles selection; if a card is armed, it is placed in that lane
    for (int l = 0; l < NUM_LANES; ++l) {
        float cy = laneBottom(l) + LANE_H / 2;
        if (!pointInRect(g_mouseX, g_mouseY, (PF_LEFT + PF_RIGHT) / 2, cy, PF_RIGHT - PF_LEFT, LANE_H)) continue;
        if (armedCard >= 0) {
            laneSelected[l] = true;
            placePlayer(armedCard, l);
            armedCard = -1;
        }
        else {
            laneSelected[l] = !laneSelected[l];
            playSfx("select");
        }
        return;
    }
}

// ------------------------------------------------------------------ update
void updateGame(float dt)
{
    gameTime += dt;

    // ---- hover (lanes + unlocked cards only)
    hoverLane = -1;
    for (int l = 0; l < NUM_LANES; ++l) {
        float cy = laneBottom(l) + LANE_H / 2;
        if (pointInRect(g_mouseX, g_mouseY, (PF_LEFT + PF_RIGHT) / 2, cy, PF_RIGHT - PF_LEFT, LANE_H)) hoverLane = l;
    }
    hoverCard = -1;
    for (int i = 0; i < 3; ++i)
        if (cardUnlocked(i) && pointInRect(g_mouseX, g_mouseY, cardX(i), CARD_Y, CARD_W, CARD_H)) hoverCard = i;
    if (armedCard >= 0 && !cardUnlocked(armedCard)) armedCard = -1;

    // ---- waves
    if (!waveRunning) {
        breakTimer -= dt;
        if (breakTimer <= 0.0f) { waveRunning = true; spawnTimer = 0.0f; }
    }
    else if (!waveQueue.empty()) {
        spawnTimer -= dt;
        if (spawnTimer <= 0.0f) {
            spawnEnemy(waveQueue.front());
            waveQueue.erase(waveQueue.begin());
            spawnTimer = WAVE_INTERVAL[waveIdx];
        }
    }
    else if (enemies.empty()) {          // wave over and its enemies are all defeated
        if (waveIdx == 2) { goToState(STATE_WIN); playSfx("win"); return; }
        waveIdx++;
        buildWave(waveIdx);
        waveRunning = false;
        breakTimer = 4.0f;
    }

    // ---- players
    for (Player& p : players) {
        p.pose.t += dt;
        p.hitFlash = glm::max(0.0f, p.hitFlash - dt * 3.0f);
        p.pose.shake = -p.pose.shake * 0.8f;
        if (fabsf(p.pose.shake) < 0.3f) p.pose.shake = 0.0f;
        if (p.jumpT >= 0) { p.jumpT += dt; if (p.jumpT >= JUMP_TIME) p.jumpT = -1.0f; }
        if (p.duckT >= 0) { p.duckT += dt; if (p.duckT >= DUCK_TIME) p.duckT = -1.0f; }

        Enemy* target = nullptr;
        for (Enemy& e : enemies)
            if (overlaps(p, e) && (target == nullptr || e.x < target->x)) target = &e;
        p.fighting = (target != nullptr);

        // movement (defense never moves)
        p.pose.moving = false;
        if (p.type != P_DEFENSE && !p.fighting) {
            p.x += PLAYER_SPEED * dt;
            float maxX = PF_RIGHT - P_FRONT[p.type];
            if (p.x > maxX) p.x = maxX; else { p.pose.moving = true; p.pose.walk += dt * 7.0f; }
        }

        // attacks start as soon as the player is placed
        if (p.type == P_DEFENSE) continue;
        float before = p.pose.attack;
        p.pose.attack += dt / P_ATK_PERIOD[p.type];
        if (p.pose.attack >= 1.0f) p.pose.attack -= 1.0f;
        float hitMoment = (p.type == P_MELEE) ? 0.62f : 0.8f;
        bool crossed = (before < hitMoment && p.pose.attack >= hitMoment);
        if (!crossed) continue;
        if (p.type == P_MELEE) {
            if (target) { target->hp -= 1; target->hitFlash = 1.0f; playSfx("whack"); }
        }
        else {
            Projectile pr;
            pr.kind = PROJ_PEBBLE; pr.lane = p.lane;
            pr.x = p.x + 20.0f; pr.y = 45.0f + jumpHeight(p); pr.vx = 330.0f; pr.startX = p.x;
            projectiles.push_back(pr);
            playSfx("shoot");
        }
    }

    // ---- enemies
    for (Enemy& e : enemies) {
        e.pose.t += dt;
        e.hitFlash = glm::max(0.0f, e.hitFlash - dt * 3.0f);

        Player* front = nullptr;    // the player that is furthest right among the ones touching us
        for (Player& p : players)
            if (overlaps(p, e) && (front == nullptr || p.x > front->x)) front = &p;
        e.fighting = (front != nullptr);

        // vehicles queue behind the vehicle in front of them (same lane)
        bool queued = false;
        for (const Enemy& o : enemies)
            if (&o != &e && o.lane == e.lane && o.x < e.x &&
                e.x - E_FRONT[e.type] < o.x + E_BACK[o.type] + 6.0f) queued = true;

        e.pose.moving = !e.fighting && !queued;
        if (e.pose.moving) {
            e.x -= ENEMY_SPEED[e.type] * dt;
            e.pose.walk += dt * ENEMY_SPEED[e.type] / 12.0f;
        }
        if (e.x - E_FRONT[e.type] <= PF_LEFT) {            // reached the metro station -> LOSE
            goToState(STATE_LOSE);
            playSfx("lose");
            return;
        }

        float before = e.pose.attack;
        e.pose.attack += dt / E_ATK_PERIOD[e.type];
        if (e.pose.attack >= 1.0f) e.pose.attack -= 1.0f;
        float hitMoment = (e.type == E_CAR) ? 0.45f : 0.5f;
        if (!(before < hitMoment && e.pose.attack >= hitMoment)) continue;

        if (e.type == E_CAR) {
            if (front) { damagePlayer(*front, ENEMY_DMG[E_CAR]); playSfx("honk"); }
        }
        else {
            Projectile pr;
            pr.lane = e.lane;
            if (e.type == E_TUKTUK) {
                pr.kind = PROJ_NOTE_HIGH; pr.x = e.x - 30.0f; pr.y = 66.0f; pr.vx = -170.0f;   // tire thrown by the driver
            }
            else {
                pr.kind = PROJ_ROCK_LOW; pr.x = e.x - 66.0f; pr.y = 7.0f; pr.vx = -190.0f;
            }
            projectiles.push_back(pr);
        }
    }

    // ---- projectiles
    for (size_t i = 0; i < projectiles.size(); ) {
        Projectile& pr = projectiles[i];
        pr.x += pr.vx * dt;
        pr.t += dt;
        pr.rot += (pr.kind == PROJ_ROCK_LOW ? 400.0f : -720.0f) * dt;
        if (pr.kind == PROJ_NOTE_HIGH && pr.y > 62.0f) pr.y -= 40.0f * dt;   // comes down to head height
        bool remove = false;

        if (pr.kind == PROJ_PEBBLE) {
            for (Enemy& e : enemies) {
                if (e.lane != pr.lane) continue;
                // hit when the pebble reached the car's front and was fired from behind
                // the car's back (so it still hits when the kid stands at the end of the lane)
                if (pr.x >= e.x - E_FRONT[e.type] && pr.startX <= e.x + E_BACK[e.type]) {
                    e.hp -= 1; e.hitFlash = 1.0f;
                    addEffect(1, pr.lane, pr.x, pr.y, 0.35f);      // pebble breaks on impact
                    playSfx("clank");
                    remove = true;
                    break;
                }
            }
            if (pr.x > PF_RIGHT) remove = true;
        }
        else {
            // 1) a defense player in this lane absorbs anything that reaches it
            Player* def = nullptr;
            for (Player& p : players)
                if (p.lane == pr.lane && p.type == P_DEFENSE && pr.x <= p.x + P_FRONT[P_DEFENSE] &&
                    pr.x >= p.x - P_BACK[P_DEFENSE] - 40.0f && (def == nullptr || p.x > def->x)) def = &p;
            if (def) {
                damagePlayer(*def, 1);
                remove = true;
            }
            else {
                // 2) other players: duck avoids HIGH attacks, jump avoids LOW attacks
                for (Player& p : players) {
                    if (p.lane != pr.lane || p.type == P_DEFENSE) continue;
                    if (pr.x < p.x - 12.0f || pr.x > p.x + 14.0f) continue;
                    bool avoided = (pr.kind == PROJ_NOTE_HIGH && p.duckT >= 0) ||
                                   (pr.kind == PROJ_ROCK_LOW && p.jumpT >= 0);
                    if (avoided) continue;
                    damagePlayer(p, 1);
                    remove = true;
                    break;
                }
            }
            if (pr.x < PF_LEFT) remove = true;
        }
        if (remove) projectiles.erase(projectiles.begin() + i); else ++i;
    }

    // ---- remove defeated characters
    for (size_t i = 0; i < players.size(); ) {
        if (players[i].hp <= 0) {
            addEffect(0, players[i].lane, players[i].x, 20.0f, 0.6f);
            playSfx("death");
            players.erase(players.begin() + i);
        }
        else ++i;
    }
    for (size_t i = 0; i < enemies.size(); ) {
        Enemy& e = enemies[i];
        if (e.hp <= 0) {
            addEffect(0, e.lane, e.x, 20.0f, 0.7f);
            Coin c; c.lane = e.lane; c.value = ENEMY_DROP[e.type]; c.t = 0;
            c.x = glm::clamp(e.x, PF_LEFT + 40.0f, PF_RIGHT - 30.0f);
            coins.push_back(c);
            playSfx("crash");
            enemies.erase(enemies.begin() + i);
        }
        else ++i;
    }

    // ---- collectibles: picked up when a player touches them
    for (size_t i = 0; i < coins.size(); ) {
        coins[i].t += dt;
        bool taken = false;
        for (Player& p : players) {
            if (p.lane != coins[i].lane) continue;
            if (coins[i].x >= p.x - P_BACK[p.type] - 8 && coins[i].x <= p.x + P_FRONT[p.type] + 8) taken = true;
            // a player standing at the end of the lane can never walk back, so it
            // also collects coins that drop behind it
            bool atEnd = (p.type != P_DEFENSE && p.x >= PF_RIGHT - P_FRONT[p.type] - 0.5f);
            if (atEnd && coins[i].x < p.x) taken = true;
        }
        if (taken) {
            g_coins += coins[i].value;
            playSfx("coin");
            coins.erase(coins.begin() + i);
        }
        else ++i;
    }

    // ---- effects
    for (size_t i = 0; i < effects.size(); ) {
        effects[i].t += dt;
        if (effects[i].t >= effects[i].life) effects.erase(effects.begin() + i); else ++i;
    }
}

// ------------------------------------------------------------------ drawing
static void drawPlayer(const Player& p)
{
    float g = laneGround(p.lane);
    pushM();
    translateM(p.x, g);
    pushM(); scaleM(CHAR_SCALE, CHAR_SCALE); drawShadow(p.type == P_DEFENSE ? 40.0f : 22.0f); popM();
    translateM(0, jumpHeight(p));
    if (p.duckT >= 0) {
        float k = sinf(PI * glm::min(p.duckT / 0.2f, 1.0f) * 0.5f);         // quick squat
        if (p.duckT > DUCK_TIME - 0.2f) k = (DUCK_TIME - p.duckT) / 0.2f;    // stand up
        scaleM(1.0f + 0.08f * k, 1.0f - 0.42f * k);
    }
    scaleM(CHAR_SCALE, CHAR_SCALE);
    // damage: the lower the health, the stronger the red
    setDamage(0.7f * (1.0f - (float)p.hp / p.maxHp) + 0.35f * p.hitFlash);
    if (p.type == P_MELEE) drawVendor(p.pose);
    else if (p.type == P_RANGED) drawKid(p.pose);
    else drawOfficer(p.pose);
    setDamage(0.0f);
    popM();

    float barX = p.x + (p.type == P_DEFENSE ? 14.0f : 0.0f);
    drawHealthBar(barX, g + 92.0f + jumpHeight(p), p.type == P_DEFENSE ? 52.0f : 40.0f, p.hp, p.maxHp);
}

static void drawEnemy(const Enemy& e)
{
    float g = laneGround(e.lane);
    float lunge = 0.0f;
    if (e.type == E_CAR && e.fighting) lunge = -12.0f * sinf(PI * glm::clamp((e.pose.attack - 0.2f) / 0.4f, 0.0f, 1.0f));
    pushM();
    translateM(e.x + lunge, g);
    scaleM(CHAR_SCALE, CHAR_SCALE);
    drawShadow(e.type == E_TUKTUK ? 50.0f : 82.0f);
    setDamage(0.7f * (1.0f - (float)e.hp / e.maxHp) + 0.35f * e.hitFlash);
    if (e.type == E_TUKTUK) drawTuktuk(e.pose);
    else if (e.type == E_MICROBUS) drawMicrobus(e.pose);
    else drawCar(e.pose);
    setDamage(0.0f);
    popM();
    float top = (e.type == E_TUKTUK) ? 96.0f : (e.type == E_MICROBUS ? 90.0f : 66.0f);
    drawHealthBar(e.x, g + top, e.type == E_TUKTUK ? 50.0f : 60.0f, e.hp, e.maxHp);
}

static void drawCard(int i)
{
    float cx = cardX(i), cy = CARD_Y;
    bool unlocked = cardUnlocked(i);
    bool hovered = (hoverCard == i);
    bool armed = (armedCard == i);
    float lift = hovered ? 4.0f : 0.0f;

    if (!unlocked) setGray(1.0f);
    Color border = armed ? Color(1.0f, 0.85f, 0.15f) * (0.8f + 0.2f * sinf(g_time * 8)) :
                   (hovered ? rgb(255, 255, 255) : rgb(60, 44, 30));
    roundBox(cx - CARD_W / 2 - 3, cy - CARD_H / 2 - 3 + lift, cx + CARD_W / 2 + 3, cy + CARD_H / 2 + 3 + lift, 10, border);
    roundBox(cx - CARD_W / 2, cy - CARD_H / 2 + lift, cx + CARD_W / 2, cy + CARD_H / 2 + lift, 8,
             unlocked ? rgb(236, 210, 150) : rgb(150, 150, 150));
    box(cx - CARD_W / 2 + 6, cy - 14 + lift, cx + CARD_W / 2 - 6, cy + CARD_H / 2 - 18 + lift, rgb(120, 170, 200));

    // portrait of the character
    pushM();
    float offX = (i == 2) ? -14.0f : 0.0f;
    translateM(cx + offX, cy - 14 + lift);
    scaleM(0.42f, 0.42f);
    Pose idle; idle.t = g_time; idle.attack = hovered ? fmodf(g_time, 1.0f) : 0.0f;
    if (i == 0) drawVendor(idle); else if (i == 1) drawKid(idle); else drawOfficer(idle);
    popM();

    drawTextCentered(CARD_NAME[i], cx, cy + CARD_H / 2 - 14 + lift, 8, rgb(60, 40, 20), 1.6f);
    pushM(); translateM(cx - 20, cy - 29 + lift); scaleM(0.6f, 0.6f); drawCoinModel(0); popM();
    drawText(std::to_string(COST[i]), cx - 10, cy - 35 + lift, 12, rgb(60, 40, 20), 2.2f);

    if (!unlocked) {          // padlock
        box(cx + 22, cy - 38, cx + 40, cy - 24, rgb(70, 70, 70));
        arc(cx + 31, cy - 24, 6, 0, 180, 2.5f, rgb(70, 70, 70));
    }
    setGray(0.0f);
}

static void drawHud()
{
    // cards panel
    roundBox(130, 622, 456, 718, 12, rgb(70, 50, 34));
    for (int i = 0; i < 3; ++i) drawCard(i);

    // wave panel
    roundBox(500, 640, 740, 716, 12, rgb(70, 50, 34));
    drawTextCentered("WAVE " + std::to_string(waveIdx + 1) + "/3", 620, 690, 16, rgb(255, 214, 90), 3);
    int left = (int)enemies.size() + (int)waveQueue.size();
    drawTextCentered("CARS LEFT " + std::to_string(left), 620, 670, 9, rgb(240, 240, 235), 1.6f);
    drawTextCentered("W/S JUMP-DUCK  ESC PAUSE", 620, 650, 7, rgb(220, 200, 170), 1.3f);

    // coins (collectibles) at the top
    roundBox(1100, 640, 1272, 716, 12, rgb(70, 50, 34));
    pushM(); translateM(1132, 678); scaleM(1.5f, 1.5f); drawCoinModel(g_time); popM();
    drawText(std::to_string(g_coins), 1160, 664, 28, rgb(255, 214, 90), 4.5f);
}

void drawGame(bool showHud)
{
    drawSkyline(g_time);
    drawStreet(g_time);
    for (int l = 0; l < NUM_LANES; ++l)
        drawLaneHighlight(l, l == hoverLane && g_state == STATE_PLAYING, laneSelected[l]);
    drawLaneMarks();
    drawEnvironment(g_time);
    drawMetroStation(g_time);

    // draw from the top lane to the bottom lane so lower lanes overlap correctly
    for (int l = NUM_LANES - 1; l >= 0; --l) {
        float g = laneGround(l);
        for (const Coin& c : coins) {
            if (c.lane != l) continue;
            pushM(); translateM(c.x, g + 14 + 4 * sinf(c.t * 4)); drawCoinModel(c.t); popM();
        }
        for (const Player& p : players) if (p.lane == l && p.type == P_DEFENSE) drawPlayer(p);
        for (const Enemy& e : enemies) if (e.lane == l) drawEnemy(e);
        for (const Player& p : players) if (p.lane == l && p.type != P_DEFENSE) drawPlayer(p);
        for (const Projectile& pr : projectiles) {
            if (pr.lane != l) continue;
            pushM(); translateM(pr.x, g + pr.y);
            if (pr.kind == PROJ_PEBBLE) drawPebble(pr.rot);
            else if (pr.kind == PROJ_NOTE_HIGH) drawTire(pr.rot);
            else drawRock(pr.rot);
            popM();
        }
        for (const Effect& f : effects) {
            if (f.lane != l) continue;
            float k = f.t / f.life;
            if (f.kind == 0) drawPuff(f.x, g + f.y, k); else drawShatter(f.x, g + f.y, k);
        }
    }

    // wave banner
    if (g_state == STATE_PLAYING && !waveRunning && breakTimer > 0.0f) {
        std::string t = (waveIdx == 2) ? "FINAL WAVE - TAXIS INCOMING!" : "WAVE " + std::to_string(waveIdx + 1) + " - HERE THEY COME!";
        box(PF_LEFT, 290, PF_RIGHT, 350, rgb(30, 22, 16));
        box(PF_LEFT, 286, PF_RIGHT, 290, rgb(255, 200, 40));
        box(PF_LEFT, 350, PF_RIGHT, 354, rgb(255, 200, 40));
        drawTextCentered(t, (PF_LEFT + PF_RIGHT) / 2, 308, 24, rgb(255, 220, 90), 3.5f);
    }

    if (showHud) drawHud();
}
