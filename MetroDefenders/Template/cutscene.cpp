// ============================================================================
//  cutscene.cpp
//
//  Story (17.5 s):
//   0 - 4    Morning in Cairo. Commuters walk to the metro station (what we defend & why).
//   4 - 7.5  A tuk-tuk (throwing tires), a microbus and a taxi arrive.
//            Microbus driver: "This square is our new microbus parking lot!"
//   7.5 - 10.5 Vendor: "No way! This station is our only way to work!"
//            The kid fires his slingshot - the pebble bounces off the tuk-tuk.
//   10.5 - 14.5 The taxi charges, the pedestrians jump back, the traffic officer
//            jumps in with his barrier and stops it. "Stop! Nobody touches our metro!"
//   14.5 - 17.5 The cars back off into the lanes -> "DEFEND THE METRO STATION!" -> game.
// ============================================================================
#include "cutscene.h"
#include "models.h"
#include "game.h"
#include "audio.h"

static float T = 0.0f;
static const float LENGTH = 17.5f;
static const float GROUND = 230.0f;     // the street line used in the cutscene
static const float S = 1.25f;           // characters are bigger in the cutscene

// sound cues fire once
static bool cueHonk1, cueShoot, cueClank, cueHonk2, cueBang, cueWhistle;

void startCutscene()
{
    T = 0.0f;
    cueHonk1 = cueShoot = cueClank = cueHonk2 = cueBang = cueWhistle = false;
}

void skipCutscene()
{
    resetGame();
    goToState(STATE_PLAYING);
}

static float clamp01(float v) { return v < 0 ? 0 : (v > 1 ? 1 : v); }
static float smooth(float a, float b, float t) { float k = clamp01((t - a) / (b - a)); return k * k * (3 - 2 * k); }
static float lerpF(float a, float b, float k) { return a + (b - a) * k; }

void updateCutscene(float dt)
{
    T += dt;
    if (!cueHonk1 && T > 4.2f)  { cueHonk1 = true; playSfx("honk"); }
    if (!cueShoot && T > 9.0f)  { cueShoot = true; playSfx("shoot"); }
    if (!cueClank && T > 9.6f)  { cueClank = true; playSfx("clank"); }
    if (!cueHonk2 && T > 10.6f) { cueHonk2 = true; playSfx("honk"); }
    if (!cueWhistle && T > 11.0f) { cueWhistle = true; playSfx("whistle"); }
    if (!cueBang && T > 12.2f)  { cueBang = true; playSfx("hit"); }
    if (T >= LENGTH) skipCutscene();
}

static void subtitle(const std::string& who, const std::string& text, Color whoColor)
{
    box(0, 0, SCREEN_W, 64, rgb(20, 16, 14));
    box(0, 64, SCREEN_W, 67, rgb(255, 200, 40));
    if (!who.empty()) {
        drawText(who + ":", 40, 26, 14, whoColor, 2.6f);
        drawText(text, 40 + textWidth(who + ": ", 14), 26, 14, rgb(245, 240, 230), 2.4f);
    }
    else drawTextCentered(text, 640, 26, 14, rgb(245, 240, 230), 2.4f);
}

static void character(int kind, float x, float y, bool faceLeft, const Pose& p, float scale, float red = 0.0f)
{
    pushM();
    translateM(x, y);
    scaleM(faceLeft ? -scale : scale, scale);
    if (kind <= 2) drawShadow(kind == 2 ? 40.0f : 22.0f);
    setDamage(red);
    switch (kind) {
    case 0: drawVendor(p); break;
    case 1: drawKid(p); break;
    case 2: drawOfficer(p); break;
    case 3: drawShadow(50); drawTuktuk(p); break;
    case 4: drawShadow(82); drawMicrobus(p); break;
    case 5: drawShadow(82); drawCar(p); break;
    }
    setDamage(0.0f);
    popM();
}

void drawCutscene()
{
    float fade = smooth(0.0f, 1.0f, T) * (1.0f - smooth(LENGTH - 1.0f, LENGTH, T));
    setFade(fade);

    drawSkyline(g_time);
    drawStreet(g_time);
    drawEnvironment(g_time);
    drawMetroStation(g_time);

    // ------------------------------------------------ vehicles (back row first)
    // microbus (back row, a bit higher on the street)
    Pose mb; mb.t = T; mb.moving = T < 6.5f || (T > 14.5f && T < 16.5f);
    float mbX = lerpF(1700, 1130, smooth(4.0f, 6.5f, T));
    mbX = lerpF(mbX, 1250, smooth(14.5f, 16.5f, T));
    mb.walk = mbX / 12.0f;
    character(4, mbX, GROUND + 110, false, mb, 1.1f);

    // taxi (back row too, charges later)
    Pose tx; tx.t = T;
    float txX = lerpF(1800, 1250, smooth(5.0f, 7.0f, T));
    float charge = smooth(10.5f, 12.2f, T);
    txX = lerpF(txX, 690, charge);
    float bounce = smooth(12.2f, 12.6f, T) - smooth(12.6f, 13.6f, T) * 0.0f;
    txX += bounce * 40.0f;
    txX = lerpF(txX, 1050, smooth(14.5f, 16.5f, T));
    float txY = lerpF(GROUND + 110, GROUND, smooth(10.5f, 11.6f, T));
    txY = lerpF(txY, GROUND + 110, smooth(14.5f, 16.0f, T));
    tx.moving = (T > 5.0f && T < 7.0f) || (T > 10.5f && T < 12.2f) || (T > 14.5f && T < 16.5f);
    tx.walk = txX / 10.0f;
    tx.attack = (T > 10.5f && T < 12.4f) ? 0.4f : 0.0f;
    character(5, txX, txY, false, tx, 1.1f);

    // tuk-tuk (front row)
    Pose tk; tk.t = T;
    float tkX = lerpF(1500, 1000, smooth(3.5f, 6.0f, T));
    tkX = lerpF(tkX, 1180, smooth(14.5f, 16.5f, T));
    tk.moving = (T > 3.5f && T < 6.0f) || T > 14.5f;
    tk.walk = tkX / 12.0f;
    tk.attack = fmodf(T, 1.0f);
    float hitRed = (T > 9.6f && T < 10.2f) ? 0.6f : 0.0f;
    character(3, tkX, GROUND, false, tk, 1.2f, hitRed);

    // tires thrown by the tuk-tuk driver
    if (T > 4.0f && T < 10.5f) {
        for (int k = 0; k < 4; ++k) {
            float age = fmodf(T * 1.0f + k * 0.25f, 1.0f);
            pushM(); translateM(tkX - 40 - age * 260, GROUND + 90); scaleM(1.3f, 1.3f); drawTire(-T * 600 + k * 40); popM();
        }
    }

    // ------------------------------------------------ pedestrians
    bool walkingIn = T < 4.0f;
    bool faceLeft = T < 4.3f;
    Pose v; v.t = T; v.moving = walkingIn;
    v.walk = T * 7.0f;
    if (T > 7.5f && T < 10.5f) v.attack = fmodf((T - 7.5f) * 1.6f, 1.0f);     // shaking the nabboot
    float vX = lerpF(700, 400, smooth(0.0f, 4.0f, T));
    float jump = 0.0f;
    if (T > 11.2f && T < 12.0f) jump = sinf((T - 11.2f) / 0.8f * PI) * 60.0f;  // jump back from the taxi
    if (T > 14.5f && T < 16.5f) jump = fabsf(sinf((T - 14.5f) * 6.0f)) * 30.0f; // cheering
    vX -= smooth(11.2f, 12.0f, T) * 70.0f;

    Pose k; k.t = T; k.moving = walkingIn; k.walk = T * 7.0f + 1.0f;
    float kX = lerpF(780, 480, smooth(0.0f, 4.0f, T)) - smooth(11.2f, 12.0f, T) * 70.0f;
    if (T > 8.2f && T < 9.4f) k.attack = (T - 8.2f) / 1.5f;                  // pull back ...
    else if (T >= 9.4f && T < 9.7f) k.attack = 0.8f + (T - 9.4f) / 0.3f * 0.2f; // ... release

    // the officer arrives with his barrier
    Pose o; o.t = T;
    float oX = lerpF(190, 520, smooth(10.8f, 12.0f, T));
    if (T > 12.2f && T < 12.8f) o.shake = sinf(T * 60.0f) * 4.0f;

    character(2, oX, GROUND - 4, false, o, S);
    character(1, kX, GROUND + jump * 0.8f, faceLeft, k, S);
    character(0, vX, GROUND + jump, faceLeft, v, S);

    // pebble flying from the slingshot to the tuk-tuk
    if (T > 9.0f && T < 9.6f) {
        float a = (T - 9.0f) / 0.6f;
        float px = lerpF(kX + 30, tkX - 30, a);
        float py = GROUND + 70 + sinf(a * PI) * 30;
        pushM(); translateM(px, py); scaleM(1.4f, 1.4f); drawPebble(T * 900); popM();
    }
    if (T > 9.6f && T < 10.0f) drawShatter(tkX - 30, GROUND + 72, (T - 9.6f) / 0.4f);
    if (T > 12.2f && T < 12.7f) drawPuff(oX + 75, GROUND + 30, (T - 12.2f) / 0.5f);

    // ------------------------------------------------ dialogue
    if (T > 4.6f && T < 7.5f)
        drawSpeechBubble(860, 420, 330, 70, mbX - 60, GROUND + 205, "THIS SQUARE IS OUR NEW", "MICROBUS PARKING LOT!");
    if (T > 7.6f && T < 10.4f)
        drawSpeechBubble(230, 400, 360, 70, vX + 10, GROUND + 125, "NO WAY! THIS STATION IS", "OUR ONLY WAY TO WORK!");
    if (T > 12.0f && T < 14.6f)
        drawSpeechBubble(330, 420, 380, 70, oX + 10, GROUND + 140, "STOP! NOBODY TOUCHES", "OUR METRO STATION!");

    if (T < 4.2f)       subtitle("", "CAIRO, 8 AM. THE METRO CARRIES MILLIONS OF PEOPLE TO WORK AND SCHOOL.", rgb(255, 255, 255));
    else if (T < 7.5f)  subtitle("MICROBUS DRIVER", "THIS SQUARE IS OUR NEW MICROBUS PARKING LOT!", rgb(120, 190, 255));
    else if (T < 10.5f) subtitle("VENDOR", "NO WAY! THIS STATION IS OUR ONLY WAY TO WORK!", rgb(255, 200, 80));
    else if (T < 14.6f) subtitle("TRAFFIC OFFICER", "STOP! NOBODY TOUCHES OUR METRO STATION! YALLA!", rgb(140, 230, 140));
    else                subtitle("", "SEND PEDESTRIANS DOWN THE LANES AND STOP THE TRAFFIC!", rgb(255, 255, 255));

    if (T > 14.6f) {
        float pop = smooth(14.6f, 15.1f, T);
        pushM(); translateM(640, 470); scaleM(pop, pop);
        drawTextCentered("DEFEND THE METRO STATION!", 3, -3, 40, rgb(60, 20, 10), 7);
        drawTextCentered("DEFEND THE METRO STATION!", 0, 0, 40, rgb(255, 210, 70), 6);
        popM();
    }

    drawText("ENTER: SKIP", 1130, 690, 10, rgb(255, 255, 255), 2);
    setFade(1.0f);
}
