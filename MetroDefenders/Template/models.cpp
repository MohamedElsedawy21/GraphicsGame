// ============================================================================
//  models.cpp : modelling of every scene component (built from primitives)
// ============================================================================
#include "models.h"

// ------------------------------------------------------------- palette
static const Color BLACK = Color(0.08f, 0.08f, 0.09f);
static const Color WHITE = Color(0.95f, 0.95f, 0.93f);

static void wheel(float cx, float cy, float r, float rot)
{
    circle(cx, cy, r, rgb(24, 24, 27));                 // tyre
    circle(cx, cy, r * 0.64f, rgb(165, 165, 172));      // rim
    circle(cx, cy, r * 0.5f, rgb(115, 115, 124));
    for (int k = 0; k < 3; ++k) {                       // spokes (rotate -> wheel spins)
        pushM();
        translateM(cx, cy);
        rotateM(rot + k * 60.0f);
        rect(0, 0, r * 1.0f, r * 0.14f, rgb(200, 200, 208));
        popM();
    }
    circle(cx, cy, r * 0.18f, rgb(60, 60, 64));
}

void drawShadow(float halfW)
{
    ellipse(0, 1, halfW, halfW * 0.16f, rgb(48, 50, 55));
}

// =================================================================== PLAYERS

// ---- MELEE: street vendor in a galabeya swinging a "nabboot" (traditional stick)
static float vendorArmAngle(float a)
{
    // idle -50 -> wind-up +70 -> fast strike -62 -> back to idle
    if (a < 0.55f) { float k = a / 0.55f; k = k * k * (3 - 2 * k); return -50.0f + 120.0f * k; }
    if (a < 0.70f) { float k = (a - 0.55f) / 0.15f; return 70.0f - 132.0f * k; }
    float k = (a - 0.70f) / 0.30f; return -62.0f + 12.0f * k;
}

void drawVendor(const Pose& p)
{
    Color robe = rgb(92, 126, 162), robeD = rgb(64, 92, 124);
    Color skin = rgb(178, 122, 82), skinD = rgb(138, 90, 58);
    Color sandal = rgb(120, 72, 40);
    float sw = p.moving ? sinf(p.walk) : 0.0f;

    // feet + sandals
    rect(-6 + sw * 5, 7, 6, 10, skin);  ellipse(-4 + sw * 5, 2.5f, 8, 3, sandal);
    rect(6 - sw * 5, 7, 6, 10, skin);   ellipse(8 - sw * 5, 2.5f, 8, 3, sandal);

    // back arm (behind the robe)
    pushM(); translateM(-9, 58); rotateM(-95.0f + sw * 22.0f);
    roundLine(0, 0, 24, 0, 8, robeD); circle(27, 0, 4.5f, skin);
    popM();

    // galabeya (flared robe)
    quad(-15, 62, 15, 62, 21, 10, -21, 10, robe);
    quad(-21, 10, -12, 10, -9, 62, -15, 62, robeD);      // shading side
    quad(-21, 10, 21, 10, 20.6f, 15, -20.6f, 15, robeD);  // hem band
    line(3, 60, 3, 34, 2, robeD);                         // front seam
    circle(3, 56, 1.4f, WHITE); circle(3, 51, 1.4f, WHITE);
    ellipse(0, 61, 16, 5, robe);                          // shoulders
    tri(-5, 63, 6, 63, 0.5f, 54, skin);                   // V collar

    // head
    rect(1, 66, 8, 6, skin);
    ball(2, 77, 11.5f, skin);
    circle(-5, 77, 3, skinD);                             // ear
    circle(8.5f, 79, 1.7f, BLACK);                        // eye
    line(5, 82.5f, 11, 82, 1.6f, rgb(40, 30, 25));        // eyebrow
    tri(12, 78, 15.5f, 73, 11, 73, skinD);                // nose
    roundLine(6, 71.5f, 13.5f, 71.5f, 3.2f, rgb(30, 24, 20)); // moustache

    // "emma" (turban)
    ellipse(1, 86, 13, 6.5f, WHITE);
    semicircle(1, 87, 11, 8, 0, WHITE);
    line(-11, 85, 13, 88, 1.4f, rgb(196, 194, 186));
    line(-10, 89, 11, 92, 1.4f, rgb(196, 194, 186));

    // front arm + nabboot (rotates around the shoulder -> swing animation)
    pushM();
    translateM(9, 58);
    rotateM(vendorArmAngle(p.attack));
    roundLine(0, 0, 22, 0, 8, robe);
    pushM();
    translateM(23, 0);
    rotateM(60);
    roundLine(-12, 0, 54, 0, 4.5f, rgb(126, 80, 40));     // stick
    rect(34, 0, 2.5f, 5.5f, rgb(84, 52, 26));             // binding rings
    rect(40, 0, 2.5f, 5.5f, rgb(84, 52, 26));
    ball(55, 0, 5.5f, rgb(104, 64, 30));                  // knob
    popM();
    circle(23, 0, 4.7f, skin);                            // hand over the stick
    popM();
}

// ---- RANGED: kid with a slingshot ("nebla")
void drawKid(const Pose& p)
{
    Color skin = rgb(160, 108, 70), skinD = rgb(124, 82, 52);
    Color shirt = rgb(200, 30, 38), shorts = rgb(40, 60, 122);
    Color wood = rgb(120, 76, 38);
    float sw = p.moving ? sinf(p.walk) : 0.0f;

    // legs + sneakers
    roundLine(-5, 28, -5 + sw * 6, 6, 6, skin);
    roundLine(5, 28, 5 - sw * 6, 6, 6, skin);
    ellipse(-3 + sw * 6, 4, 7, 4, WHITE); line(-10 + sw * 6, 1, 4 + sw * 6, 1, 2, rgb(60, 60, 60));
    ellipse(7 - sw * 6, 4, 7, 4, WHITE);  line(0 - sw * 6, 1, 14 - sw * 6, 1, 2, rgb(60, 60, 60));

    // shorts + football shirt
    quad(-11, 36, 11, 36, 12, 23, -12, 23, shorts);
    shadeBox(-11, 34, 11, 58, shirt);
    box(-11, 49, 11, 51, WHITE);
    drawText("7", -2, 37, 10, WHITE, 2.0f);

    // head + backwards cap
    rect(1, 60, 6, 5, skin);
    ball(1, 68, 10.5f, skin);
    semicircle(0, 71, 10.5f, 7, 0, BLACK);              // hair
    semicircle(0, 72.5f, 11, 8, 0, rgb(30, 130, 70));    // cap
    rect(-12, 73, 9, 3, rgb(20, 96, 50));                // brim (backwards)
    circle(0, 80.5f, 1.6f, rgb(20, 96, 50));
    circle(-4, 68, 2.6f, skinD);                         // ear
    circle(7, 69.5f, 1.6f, BLACK);                       // eye
    line(5, 64, 9, 63.5f, 1.4f, rgb(110, 50, 40));       // smile

    // slingshot pull animation: 0..0.8 pull back, 0.8..1 snap
    float a = p.attack;
    float pull = a < 0.8f ? a / 0.8f : 1.0f - (a - 0.8f) / 0.2f;
    float handX = 17.0f - pull * 24.0f;

    // front arm holding the Y fork
    roundLine(7, 55, 22, 51, 5.5f, skin);
    roundLine(24, 43, 24, 51, 3.6f, wood);
    roundLine(24, 51, 20, 60, 3.0f, wood);
    roundLine(24, 51, 29, 60, 3.0f, wood);
    // rubber bands to the pouch held by the back hand
    line(20, 60, handX, 53, 1.3f, rgb(70, 40, 30));
    line(29, 60, handX, 53, 1.3f, rgb(70, 40, 30));
    if (a < 0.8f) circle(handX, 53, 3.2f, rgb(130, 130, 136));  // pebble loaded
    circle(24, 51, 3.6f, skin);
    // back arm pulling
    roundLine(-6, 56, handX, 53, 5.5f, skin);
    circle(handX, 53, 3.4f, skin);
}

// ---- DEFENSE: traffic officer with a red/white metal barrier
void drawOfficer(const Pose& p)
{
    Color uni = rgb(238, 238, 232), uniD = rgb(196, 198, 202);
    Color gold = rgb(222, 180, 60), skin = rgb(150, 100, 66), skinD = rgb(118, 78, 50);
    Color steel = rgb(150, 152, 158);

    // boots + trousers
    box(-13, 0, -3, 12, BLACK); ellipse(-4, 3, 6, 3, BLACK);
    box(3, 0, 13, 12, BLACK);   ellipse(12, 3, 6, 3, BLACK);
    shadeBox(-12, 12, -2, 45, uni);
    shadeBox(2, 12, 12, 45, uni);
    line(-11, 14, -11, 44, 1.5f, BLACK);

    // raised hand ("STOP!") every few seconds
    float cyc = fmodf(p.t, 3.0f);
    float raise = cyc < 1.2f ? sinf(cyc / 1.2f * PI) : 0.0f;
    pushM(); translateM(-12, 76); rotateM(-100.0f + raise * 210.0f);
    roundLine(0, 0, 26, 0, 7.5f, uniD);
    circle(29, 0, 5.2f, WHITE);                      // white glove
    popM();

    // jacket
    shadeBox(-17, 42, 17, 81, uni);
    box(-17, 44, 17, 49, BLACK);                     // belt
    rect(0, 46.5f, 6, 5, gold);                      // buckle
    line(-15, 79, 13, 49, 3, BLACK);                 // shoulder strap
    circle(0, 56, 1.5f, gold); circle(0, 63, 1.5f, gold); circle(0, 70, 1.5f, gold);
    circle(-9, 70, 3.2f, gold); circle(-9, 70, 1.5f, rgb(170, 30, 30));  // badge
    rect(-13, 80, 9, 3.5f, BLACK); rect(13, 80, 9, 3.5f, BLACK);         // epaulettes
    line(-16, 80, -10, 80, 1, gold); line(10, 80, 16, 80, 1, gold);
    tri(-5, 81, 6, 81, 0.5f, 74, uniD);                                  // collar

    // head: sunglasses, moustache, beret
    rect(1, 84, 8, 6, skin);
    ball(2, 92, 11.5f, skin);
    circle(-5, 92, 3, skinD);
    box(3, 91, 14, 95.5f, BLACK);                     // sunglasses
    line(-4, 94, 4, 94, 1.5f, BLACK);
    tri(12.5f, 91, 15.5f, 87, 11.5f, 87, skinD);
    roundLine(5, 85, 13.5f, 85, 3.8f, BLACK);         // moustache
    ellipseRot(3, 102, 13.5f, 5, -8, BLACK);          // beret
    semicircle(1, 101, 11, 6, 0, BLACK);
    circle(7, 103, 2, gold);

    // front arm gripping the barrier
    roundLine(13, 76, 26, 58, 7.5f, uni);

    // barrier (shakes when it absorbs a hit)
    pushM();
    translateM(p.shake, 0);
    box(18, 0, 34, 4, steel); box(46, 0, 62, 4, steel);    // feet
    box(24, 0, 28, 66, steel); box(52, 0, 56, 66, steel);  // posts
    for (int i = 0; i < 4; ++i) {
        float x0 = 28 + i * 6.0f;
        box(x0, 24, x0 + 6, 60, (i % 2 == 0) ? rgb(210, 34, 34) : WHITE);
    }
    box(24, 58, 56, 62, steel); box(24, 22, 56, 26, steel); // rails
    circle(26, 64, 2.5f, rgb(255, 150, 30));               // reflectors
    circle(54, 64, 2.5f, rgb(255, 150, 30));
    popM();
    circle(27, 57, 5.2f, WHITE);                           // glove on the rail
}

// =================================================================== ENEMIES

// ---- EASY: black tuk-tuk whose driver throws tires at head height (attacks from ABOVE)
void drawTuktuk(const Pose& p)
{
    Color red = rgb(34, 34, 38), redD = rgb(18, 18, 20);
    Color chrome = rgb(205, 205, 212), canvas = rgb(70, 70, 78);
    Color glass = rgb(150, 200, 225), skin = rgb(150, 98, 62);
    float bob = p.moving ? sinf(p.walk * 2.0f) * 1.2f : 0.0f;
    float rot = p.walk * 57.0f;

    pushM();
    translateM(0, bob);
    // canopy posts + roof
    line(-20, 52, -25, 82, 3, canvas);
    line(40, 40, 42, 84, 3, canvas);
    shadeBox(-32, 80, 50, 90, canvas);
    semicircle(9, 90, 41, 5, 0, canvas);
    // driver inside
    box(-14, 44, 6, 62, rgb(70, 74, 90));
    ball(-6, 66, 8, skin);
    semicircle(-6, 70, 8.5f, 5, 0, rgb(240, 240, 240));    // white cap
    circle(-12, 67, 1.3f, BLACK);
    line(-12, 56, -26, 60, 3.5f, rgb(70, 74, 90));
    // rear body (passenger tub) + rounded back
    shadeBox(-18, 14, 40, 46, red);
    semicircle(40, 30, 16, 16, -90, red);
    // front nose + windshield + handlebar
    quad(-18, 14, -42, 18, -36, 50, -18, 55, red);
    quad(-20, 55, -35, 52, -29, 79, -20, 80, glass);
    line(-29, 79, -20, 80, 2, canvas);
    line(-20, 58, -30, 61, 2.5f, chrome);
    semicircle(-35, 14, 13, 10, 0, redD);                  // fender
    circle(-38, 38, 5, chrome); circle(-38, 38, 3.6f, rgb(255, 236, 140)); // headlight

    popM();

    wheel(-35, 11, 11, rot);
    wheel(30, 12, 12, rot);
}

// ---- MEDIUM: plain white Cairo microbus spitting gravel from its wheels (attacks from BELOW)
void drawMicrobus(const Pose& p)
{
    Color body = rgb(236, 236, 228);
    Color glass = rgb(140, 190, 216), dark = rgb(44, 44, 50), skin = rgb(150, 100, 66);
    float bob = p.moving ? sinf(p.walk * 2.0f) * 1.0f : 0.0f;
    float rot = p.walk * 45.0f;

    pushM();
    translateM(0, bob);
    // body
    shadeBox(-78, 16, 82, 84, body);
    quad(-78, 50, -64, 50, -60, 80, -72, 80, glass);         // windshield
    for (int i = 0; i < 4; ++i) {                           // side windows
        float x0 = -54 + i * 33.0f;
        box(x0, 52, x0 + 27, 79, glass);
        line(x0, 52, x0 + 27, 52, 1.5f, dark);
    }
    ball(-66, 64, 6, skin);                                 // driver
    roundLine(-71, 58, -84, 52, 4, skin);                   // arm out of the window, waving
    line(-22, 18, -22, 82, 1.5f, rgb(170, 170, 170));        // sliding door
    line(12, 18, 12, 82, 1.5f, rgb(170, 170, 170));
    rect(6, 46, 7, 2.5f, dark);
    semicircle(-50, 16, 16, 16, 0, dark);                   // wheel arches
    semicircle(52, 16, 16, 16, 0, dark);
    box(-82, 14, -72, 24, rgb(70, 70, 76));                 // bumpers
    box(78, 14, 86, 24, rgb(70, 70, 76));
    rect(-76, 28, 4, 8, rgb(255, 230, 120));                // headlight
    rect(82, 30, 3, 8, rgb(220, 40, 40));                   // tail light
    line(-76, 70, -84, 72, 2, dark); rect(-86, 72, 4, 9, dark); // mirror
    popM();

    wheel(-50, 14, 13, rot);
    wheel(52, 14, 13, rot);
}

// ---- HARD: speeding black & white Cairo taxi (melee, 2 damage)
void drawCar(const Pose& p)
{
    Color blk = rgb(28, 28, 32), wht = rgb(240, 240, 236);
    Color glass = rgb(140, 186, 214), skin = rgb(150, 100, 66);
    float rot = p.walk * 70.0f;

    // cabin
    quad(-36, 38, 42, 38, 30, 63, -22, 63, blk);
    quad(-31, 42, 1, 42, 1, 59, -19, 59, glass);
    quad(5, 42, 37, 42, 27, 59, 5, 59, glass);
    ball(-12, 51, 6.5f, skin);                       // driver
    box(-16, 51, -6, 54, BLACK);                     // sunglasses
    // roof sign
    box(-12, 63, 18, 71, wht);
    drawText("TAXI", -10, 64.5f, 5, blk, 1.2f);
    // lower body
    shadeBox(-72, 14, 72, 38, blk);
    quad(-72, 38, -36, 38, -36, 42, -72, 40, blk);   // hood
    box(-34, 16, 34, 37, wht);                       // white doors
    for (int i = 0; i < 11; ++i)                     // checker strip
        box(-33 + i * 6.0f, 31, -27 + i * 6.0f, 35, (i % 2) ? blk : wht);
    line(0, 16, 0, 37, 1.5f, blk);
    rect(-10, 27, 6, 2, blk); rect(22, 27, 6, 2, blk);
    // angry headlight
    circle(-68, 30, 5, rgb(255, 230, 110));
    tri(-75, 37, -61, 37, -61, 31, blk);             // "angry" eyelid
    rect(71, 28, 3, 8, rgb(220, 40, 40));
    semicircle(-46, 14, 15, 14, 0, rgb(14, 14, 16));
    semicircle(46, 14, 15, 14, 0, rgb(14, 14, 16));

    wheel(-46, 13, 13, rot);
    wheel(46, 13, 13, rot);
}

// =================================================================== SMALL THINGS

// Egyptian 1-pound coin (silver ring + gold centre), bobbing and glinting
void drawCoinModel(float t)
{
    float s = 0.75f + 0.25f * fabsf(cosf(t * 3.0f));   // "spinning" by scaling x
    pushM();
    scaleM(s, 1.0f);
    circle(0, 0, 12, rgb(120, 120, 126));
    circle(0, 0, 11, rgb(196, 198, 204));
    circle(0, 0, 7.5f, rgb(226, 180, 60));
    drawText("1", -1.8f, -3.5f, 7, rgb(150, 110, 30), 1.6f);
    tri(-8, 4, -4, 8, -6, 2, WHITE);
    popM();
}

void drawPebble(float rot)
{
    pushM(); rotateM(rot);
    ball(0, 0, 5, rgb(140, 140, 146));
    circle(1.5f, -1.5f, 1.6f, rgb(90, 90, 96));
    popM();
}

// spinning tire thrown by the tuk-tuk (HIGH attack)
void drawTire(float rot)
{
    pushM(); rotateM(rot);
    circle(0, 0, 10, rgb(24, 24, 26));
    for (int i = 0; i < 8; ++i) {                 // tread blocks
        pushM(); rotateM(i * 45.0f); rect(9.5f, 0, 3, 4, rgb(50, 50, 54)); popM();
    }
    circle(0, 0, 5.5f, rgb(150, 150, 156));
    circle(0, 0, 2.2f, rgb(70, 70, 74));
    popM();
}

// rolling gravel stone (LOW attack)
void drawRock(float rot)
{
    pushM(); rotateM(rot);
    ellipse(0, 0, 8, 6.5f, rgb(122, 100, 80));
    ellipse(2, 2, 5, 4, rgb(150, 126, 100));
    circle(-3, -2, 1.6f, rgb(80, 64, 50));
    popM();
}

void drawHealthBar(float cx, float y, float w, int hp, int maxHp)
{
    float h = 7.0f;
    box(cx - w / 2 - 2, y - 2, cx + w / 2 + 2, y + h + 2, rgb(18, 18, 20));
    float ratio = (float)hp / (float)maxHp;
    Color fill = ratio > 0.6f ? rgb(60, 210, 70) : (ratio > 0.3f ? rgb(240, 200, 40) : rgb(230, 50, 40));
    float segW = w / maxHp;
    for (int i = 0; i < maxHp; ++i) {
        float x0 = cx - w / 2 + i * segW;
        box(x0 + 0.8f, y, x0 + segW - 0.8f, y + h, i < hp ? fill : rgb(70, 70, 74));
    }
}

void drawPuff(float x, float y, float k)
{
    float r = 8 + k * 26;
    Color c = lerpColor(rgb(235, 230, 215), rgb(120, 120, 124), k);
    circle(x - r * 0.6f, y + r * 0.2f, r * 0.7f, c);
    circle(x + r * 0.6f, y + r * 0.3f, r * 0.65f, c);
    circle(x, y + r * 0.7f, r * 0.8f, c);
}

void drawShatter(float x, float y, float k)
{
    for (int i = 0; i < 6; ++i) {
        float a = i * 60.0f * PI / 180.0f + 0.4f;
        float d = 4 + k * 22;
        pushM(); translateM(x + cosf(a) * d, y + sinf(a) * d); rotateM(k * 400 + i * 50);
        tri(-3, -2, 3, -2, 0, 3, rgb(150, 150, 156));
        popM();
    }
    circle(x, y, 6 * (1 - k), rgb(255, 240, 160));
}

// =================================================================== SCENE

// Cairo skyline: hazy sky, pyramids, apartment blocks with dishes & water tanks
void drawSkyline(float t)
{
    // sky (horizontal strips -> gradient)
    for (int i = 0; i < 12; ++i) {
        float y0 = 600 + i * 10.0f;
        Color c = lerpColor(rgb(250, 214, 160), rgb(120, 176, 222), i / 11.0f);
        box(0, y0, SCREEN_W, y0 + 10.5f, c);
    }
    // distant pyramids in the haze
    tri(840, 600, 905, 600, 872, 632, rgb(226, 196, 150));
    tri(885, 600, 960, 600, 922, 640, rgb(232, 202, 156));

    // apartment blocks
    struct B { float x, w, h; Color c; };
    B bl[] = {
        { 120, 70, 92, rgb(214, 170, 120) }, { 192, 60, 70, rgb(190, 140, 100) },
        { 255, 80, 104, rgb(226, 196, 150) }, { 338, 56, 80, rgb(180, 120, 90) },
        { 396, 74, 98, rgb(210, 180, 140) }, { 472, 60, 66, rgb(196, 150, 110) },
        { 534, 80, 88, rgb(220, 186, 136) }, { 770, 64, 84, rgb(200, 160, 116) },
        { 960, 70, 76, rgb(214, 176, 126) }, { 1034, 80, 102, rgb(186, 136, 98) },
        { 1116, 66, 82, rgb(222, 190, 144) }, { 1184, 96, 94, rgb(200, 150, 110) }
    };
    for (const B& b : bl) {
        shadeBox(b.x, 600, b.x + b.w, 600 + b.h, b.c);
        Color win = rgb(90, 80, 70);
        for (float wy = 612; wy < 600 + b.h - 10; wy += 18)
            for (float wx = b.x + 8; wx < b.x + b.w - 10; wx += 16) {
                box(wx, wy, wx + 8, wy + 10, win);
                box(wx - 1, wy - 2, wx + 9, wy, rgb(150, 130, 110));   // balcony ledge
            }
        // roof: water tank + satellite dish
        box(b.x + 8, 600 + b.h, b.x + 22, 600 + b.h + 9, rgb(80, 80, 86));
        semicircle(b.x + b.w - 14, 600 + b.h + 6, 7, 4, 200, rgb(230, 230, 230));
        line(b.x + b.w - 14, 600 + b.h, b.x + b.w - 14, 600 + b.h + 5, 1.5f, rgb(120, 120, 120));
    }
    // laundry line between two blocks
    line(338, 660, 396, 664, 1, rgb(60, 60, 60));
    rect(352, 655, 8, 9, rgb(230, 80, 80)); rect(368, 656, 9, 10, rgb(80, 140, 230)); rect(384, 658, 7, 8, WHITE);
    (void)t;
}

void drawStreet(float t)
{
    (void)t;
    // asphalt
    box(0, LANE_BOTTOM, SCREEN_W, LANE_BOTTOM + NUM_LANES * LANE_H, rgb(72, 74, 80));

    // sidewalks (one plain colour)
    float topCurb = LANE_BOTTOM + NUM_LANES * LANE_H;
    box(0, 0, SCREEN_W, LANE_BOTTOM, rgb(198, 182, 150));
    box(0, topCurb, SCREEN_W, 600, rgb(198, 182, 150));
}

void drawLaneHighlight(int lane, bool hovered, bool selected)
{
    float y0 = laneBottom(lane), y1 = y0 + LANE_H;
    if (selected) {
        box(PF_LEFT, y0, PF_RIGHT, y1, rgb(122, 116, 70));
        box(PF_LEFT, y0, PF_RIGHT, y0 + 4, rgb(255, 214, 40));
        box(PF_LEFT, y1 - 4, PF_RIGHT, y1, rgb(255, 214, 40));
    }
    else if (hovered) {
        box(PF_LEFT, y0, PF_RIGHT, y1, rgb(104, 108, 118));
    }
    if (hovered) {
        box(PF_LEFT, y0, PF_RIGHT, y0 + 2, rgb(255, 255, 255));
        box(PF_LEFT, y1 - 2, PF_RIGHT, y1, rgb(255, 255, 255));
    }
}

void drawLaneMarks()
{
    // manholes & cracks
    float mh[4][2] = { { 420, 140 }, { 900, 268 }, { 640, 395 }, { 1120, 510 } };
    for (auto& m : mh) {
        circle(m[0], m[1], 12, rgb(52, 52, 56));
        circle(m[0], m[1], 10, rgb(84, 84, 90));
        line(m[0] - 8, m[1], m[0] + 8, m[1], 1.5f, rgb(52, 52, 56));
        line(m[0], m[1] - 8, m[0], m[1] + 8, 1.5f, rgb(52, 52, 56));
    }
    line(300, 330, 330, 322, 1.5f, rgb(52, 54, 58)); line(330, 322, 345, 330, 1.5f, rgb(52, 54, 58));
    line(760, 120, 790, 128, 1.5f, rgb(52, 54, 58));
    // dashed lane separators
    for (int i = 1; i < NUM_LANES; ++i) {
        float y = laneBottom(i);
        for (float x = PF_LEFT + 10; x < PF_RIGHT; x += 70)
            box(x, y - 2, x + 40, y + 2, rgb(236, 236, 230));
    }
}

// ---- environment elements -------------------------------------------------
static void kiosk(float x)   // "koshk" corner shop
{
    shadeBox(x, 4, x + 86, 52, rgb(40, 120, 92));
    box(x + 8, 14, x + 60, 46, rgb(60, 46, 36));           // window opening
    Color snack[5] = { rgb(240, 60, 60), rgb(250, 200, 40), rgb(60, 160, 240), rgb(240, 130, 40), rgb(140, 220, 90) };
    for (int r = 0; r < 2; ++r)                            // shelves of snacks
        for (int i = 0; i < 6; ++i)
            box(x + 11 + i * 8.0f, 17 + r * 14.0f, x + 17 + i * 8.0f, 27 + r * 14.0f, snack[(i + r * 2) % 5]);
    box(x + 64, 6, x + 82, 46, rgb(235, 235, 235));        // drinks fridge
    box(x + 66, 10, x + 80, 42, rgb(160, 210, 230));
    for (int i = 0; i < 3; ++i) box(x + 68, 13 + i * 10.0f, x + 78, 16 + i * 10.0f, rgb(200, 40, 40));
    for (int i = 0; i < 6; ++i)                            // striped awning
        quad(x - 4 + i * 16.0f, 52, x + 12 + i * 16.0f, 52, x + 12 + i * 16.0f, 64, x - 4 + i * 16.0f, 64,
            (i % 2) ? rgb(240, 240, 235) : rgb(210, 40, 40));
    for (int i = 0; i < 6; ++i) semicircle(x + 4 + i * 16.0f, 52, 8, 4, 180, (i % 2) ? rgb(240, 240, 235) : rgb(210, 40, 40));
}

static void fulCart(float x, float t)   // "arabeyet ful" with the copper "idra" pot
{
    box(x - 34, 16, x + 34, 34, rgb(150, 96, 52));          // wooden cart
    for (int i = 0; i < 4; ++i) line(x - 34, 20 + i * 4.0f, x + 34, 20 + i * 4.0f, 1, rgb(120, 76, 40));
    box(x - 38, 34, x + 38, 38, rgb(120, 76, 40));
    line(x + 34, 30, x + 54, 40, 3, rgb(120, 76, 40));      // handle
    ball(x - 8, 50, 14, rgb(200, 116, 56));                // idra pot
    box(x - 13, 60, x - 3, 68, rgb(200, 116, 56));
    ellipse(x - 8, 69, 8, 2.5f, rgb(170, 94, 40));
    float steam = fmodf(t, 2.0f) / 2.0f;
    circle(x - 8 + sinf(t * 3) * 2, 74 + steam * 14, 2 + steam * 2.5f, rgb(230, 230, 230));
    for (int i = 0; i < 3; ++i) ellipse(x + 16, 40 + i * 4.0f, 10, 3, rgb(226, 190, 130)); // baladi bread
    wheel(x - 22, 10, 9, 0); wheel(x + 22, 10, 9, 0);
}

static void trashBin(float x)
{
    shadeBox(x - 12, 6, x + 12, 40, rgb(40, 110, 60));
    box(x - 14, 40, x + 14, 45, rgb(30, 86, 46));
    line(x - 6, 12, x - 6, 36, 1.5f, rgb(30, 86, 46));
    line(x + 6, 12, x + 6, 36, 1.5f, rgb(30, 86, 46));
    circle(x - 8, 5, 4, BLACK); circle(x + 8, 5, 4, BLACK);
    ellipse(x + 22, 10, 9, 8, rgb(30, 30, 34));             // trash bag
    tri(x + 20, 17, x + 26, 17, x + 23, 22, rgb(30, 30, 34));
}

static void streetCat(float x, float t)
{
    Color fur = rgb(214, 140, 60), furD = rgb(170, 100, 40);
    pushM(); translateM(x + 10, 8); rotateM(30 + sinf(t * 2.5f) * 25);   // swaying tail
    roundLine(0, 0, 18, 0, 3.5f, furD);
    popM();
    ellipse(x, 14, 11, 9, fur);
    circle(x - 7, 26, 7, fur);
    tri(x - 13, 30, x - 9, 30, x - 12, 37, fur);
    tri(x - 5, 30, x - 1, 30, x - 3, 37, fur);
    circle(x - 9.5f, 27, 1.2f, BLACK); circle(x - 5, 27, 1.2f, BLACK);
    line(x - 2, 13, x + 4, 19, 2, furD);
}

static void bench(float x)
{
    box(x - 30, 22, x + 30, 26, rgb(150, 100, 56));
    box(x - 30, 30, x + 30, 34, rgb(150, 100, 56));
    box(x - 30, 40, x + 30, 44, rgb(150, 100, 56));
    box(x - 26, 4, x - 22, 44, rgb(60, 60, 64));
    box(x + 22, 4, x + 26, 44, rgb(60, 60, 64));
}

void drawEnvironment(float t)
{

    kiosk(170);
    bench(560);
    streetCat(650, t);
    trashBin(760);
    fulCart(1030, t + 0.7f);
    trashBin(1170);
}

// Home base: Cairo metro station entrance (distinct colours + red metro band)
void drawMetroStation(float t)
{
    Color stone = rgb(220, 202, 166), stoneD = rgb(190, 170, 134);
    Color metroRed = rgb(196, 30, 44);

    shadeBox(0, 0, 116, 604, stone);
    for (float y = 20; y < 600; y += 28) line(0, y, 116, y, 1.2f, stoneD);
    box(0, 598, 120, 612, rgb(150, 130, 100));                 // cornice
    box(104, 0, 116, 604, metroRed);                           // red metro band

    // one stair entrance for every lane
    for (int i = 0; i < NUM_LANES; ++i) {
        float y0 = laneBottom(i);
        box(14, y0 + 6, 98, y0 + 86, rgb(150, 132, 100));      // frame
        semicircle(56, y0 + 86, 42, 26, 0, rgb(150, 132, 100));
        box(20, y0 + 8, 92, y0 + 84, rgb(38, 34, 32));         // dark opening
        semicircle(56, y0 + 84, 36, 22, 0, rgb(38, 34, 32));
        for (int s = 0; s < 6; ++s) {                          // stairs going down
            float c = 0.55f - s * 0.07f;
            box(22 + s * 3.0f, y0 + 10 + s * 11.0f, 90 - s * 3.0f, y0 + 16 + s * 11.0f, Color(c, c * 0.95f, c * 0.9f));
        }
        line(24, y0 + 12, 50, y0 + 74, 3, rgb(40, 150, 90));    // green railings
        line(88, y0 + 12, 62, y0 + 74, 3, rgb(40, 150, 90));
        float lamp = 0.8f + 0.2f * sinf(t * 3 + i);
        circle(56, y0 + 100, 9, metroRed);                      // small "M" sign
        drawTextCentered("M", 56, y0 + 96, 8, Color(1, 1, 1) * lamp, 1.8f);
    }

    // big station sign on top
    box(55, 612, 61, 640, rgb(80, 80, 86));
    circle(58, 664, 28, WHITE);
    circle(58, 664, 25, metroRed);
    drawTextCentered("M", 58, 650, 28, WHITE, 5);
    box(6, 612, 110, 630, rgb(30, 60, 140));
    drawTextCentered("METRO", 58, 616, 10, WHITE, 2);
}

void drawSpeechBubble(float x, float y, float w, float h, float tailX, float tailY, const std::string& l1, const std::string& l2)
{
    roundBox(x - 2, y - 2, x + w + 2, y + h + 2, 12, BLACK);
    float bx = glm::clamp(tailX, x + 20, x + w - 20);
    tri(bx - 10, y + 1, bx + 10, y + 1, tailX, tailY, BLACK);
    roundBox(x, y, x + w, y + h, 10, WHITE);
    tri(bx - 7, y + 3, bx + 7, y + 3, tailX + (tailX > bx ? -2 : 2), tailY + 4, WHITE);
    float lh = 13;
    if (l2.empty()) drawTextCentered(l1, x + w / 2, y + h / 2 - lh / 2, lh, BLACK, 2.2f);
    else {
        drawTextCentered(l1, x + w / 2, y + h / 2 + 4, lh, BLACK, 2.2f);
        drawTextCentered(l2, x + w / 2, y + h / 2 - lh - 4, lh, BLACK, 2.2f);
    }
}
