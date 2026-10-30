// ============================================================================
//  menus.cpp
// ============================================================================
#include "menus.h"
#include "models.h"
#include "game.h"
#include "audio.h"

struct Button {
    float cx, cy, w, h;
    std::string label;
    int id;
};

enum ButtonId {
    BTN_START, BTN_SOUND, BTN_EXIT,
    BTN_VOL_DOWN, BTN_VOL_UP, BTN_MUTE, BTN_BACK,
    BTN_CONTINUE, BTN_RESTART, BTN_MAINMENU
};

static std::vector<Button> currentButtons()
{
    std::vector<Button> b;
    switch (g_state) {
    case STATE_MAIN_MENU:
        b.push_back({ 640, 330, 320, 60, "START GAME", BTN_START });
        b.push_back({ 640, 250, 320, 60, "SOUND SETTINGS", BTN_SOUND });
        b.push_back({ 640, 170, 320, 60, "EXIT", BTN_EXIT });
        break;
    case STATE_SOUND_MENU:
        b.push_back({ 470, 330, 90, 60, "-", BTN_VOL_DOWN });
        b.push_back({ 810, 330, 90, 60, "+", BTN_VOL_UP });
        b.push_back({ 640, 240, 260, 60, isMuted() ? "UNMUTE" : "MUTE", BTN_MUTE });
        b.push_back({ 640, 160, 260, 60, "BACK", BTN_BACK });
        break;
    case STATE_PAUSED:
        b.push_back({ 640, 380, 320, 60, "CONTINUE", BTN_CONTINUE });
        b.push_back({ 640, 300, 320, 60, "RESTART", BTN_RESTART });
        b.push_back({ 640, 220, 320, 60, "MAIN MENU", BTN_MAINMENU });
        break;
    case STATE_WIN:
    case STATE_LOSE:
        b.push_back({ 640, 300, 320, 60, "RESTART", BTN_RESTART });
        b.push_back({ 640, 220, 320, 60, "MAIN MENU", BTN_MAINMENU });
        break;
    default: break;
    }
    return b;
}

static int hoveredId = -1;

void updateMenus(float dt)
{
    (void)dt;
    int before = hoveredId;
    hoveredId = -1;
    for (const Button& b : currentButtons())
        if (pointInRect(g_mouseX, g_mouseY, b.cx, b.cy, b.w, b.h)) hoveredId = b.id;
    if (hoveredId != -1 && hoveredId != before) playSfx("tick");
}

void menuMouseClick(int button)
{
    if (button != GLFW_MOUSE_BUTTON_LEFT) return;
    for (const Button& b : currentButtons()) {
        if (!pointInRect(g_mouseX, g_mouseY, b.cx, b.cy, b.w, b.h)) continue;
        playSfx("select");
        switch (b.id) {
        case BTN_START:    goToState(STATE_CUTSCENE); break;
        case BTN_SOUND:    goToState(STATE_SOUND_MENU); break;
        case BTN_EXIT:     glfwSetWindowShouldClose(g_window, true); break;
        case BTN_VOL_DOWN: volumeDown(); break;
        case BTN_VOL_UP:   volumeUp(); break;
        case BTN_MUTE:     toggleMute(); break;
        case BTN_BACK:     goToState(STATE_MAIN_MENU); break;
        case BTN_CONTINUE: goToState(STATE_PLAYING); break;
        case BTN_RESTART:  resetGame(); goToState(STATE_PLAYING); break;
        case BTN_MAINMENU: goToState(STATE_MAIN_MENU); break;
        }
        return;
    }
}

static void drawButton(const Button& b)
{
    bool hov = (hoveredId == b.id);
    float lift = hov ? 3.0f : 0.0f;
    roundBox(b.cx - b.w / 2, b.cy - b.h / 2 - 5, b.cx + b.w / 2, b.cy + b.h / 2 - 5, 14, rgb(90, 20, 26));    // 3D base
    roundBox(b.cx - b.w / 2, b.cy - b.h / 2 + lift, b.cx + b.w / 2, b.cy + b.h / 2 + lift, 14,
             hov ? rgb(240, 70, 70) : rgb(196, 30, 44));
    roundBox(b.cx - b.w / 2 + 5, b.cy + 2 + lift, b.cx + b.w / 2 - 5, b.cy + b.h / 2 - 5 + lift, 10,
             hov ? rgb(250, 110, 110) : rgb(214, 60, 70));
    drawTextCentered(b.label, b.cx, b.cy - 10 + lift, 20, rgb(255, 245, 225), 3.4f);
}

static void panel(float x0, float y0, float x1, float y1)
{
    roundBox(x0 - 6, y0 - 6, x1 + 6, y1 + 6, 22, rgb(40, 28, 20));
    roundBox(x0, y0, x1, y1, 18, rgb(232, 210, 160));
    // little Islamic-style star pattern along the top border
    for (float x = x0 + 30; x < x1 - 20; x += 40) {
        pushM(); translateM(x, y1 - 14);
        rect(0, 0, 10, 10, rgb(30, 92, 172));
        rectRot(0, 0, 10, 10, 45, rgb(30, 92, 172));
        circle(0, 0, 2.5f, rgb(232, 210, 160));
        popM();
    }
}

// animated street behind the main menu
static void drawMenuBackground()
{
    drawSkyline(g_time);
    drawStreet(g_time);
    drawLaneMarks();
    drawEnvironment(g_time);
    drawMetroStation(g_time);

    // traffic driving by + commuters walking to the metro
    float loop = 1600.0f;
    Pose car; car.t = g_time; car.moving = true; car.walk = g_time * 6.0f;
    pushM(); translateM(1400 - fmodf(g_time * 90.0f, loop), laneGround(3)); scaleM(CHAR_SCALE, CHAR_SCALE); drawCar(car); popM();
    pushM(); translateM(1400 - fmodf(g_time * 40.0f + 700, loop), laneGround(0)); scaleM(CHAR_SCALE, CHAR_SCALE); drawMicrobus(car); popM();
    Pose ped; ped.t = g_time; ped.moving = true; ped.walk = g_time * 7.0f;
    pushM(); translateM(1300 - fmodf(g_time * 35.0f, 1200), laneGround(2)); scaleM(-CHAR_SCALE, CHAR_SCALE); drawVendor(ped); popM();
    pushM(); translateM(1360 - fmodf(g_time * 35.0f, 1200), laneGround(2)); scaleM(-CHAR_SCALE, CHAR_SCALE); drawKid(ped); popM();
}

static void drawTitle()
{
    float bob = sinf(g_time * 2.0f) * 4.0f;
    drawTextCentered("METRO DEFENDERS", 643, 472 + bob, 44, rgb(60, 20, 10), 8);
    drawTextCentered("METRO DEFENDERS", 640, 475 + bob, 44, rgb(255, 210, 70), 7);
    drawTextCentered("THE EGYPTIAN STREET", 640, 430, 20, rgb(120, 30, 30), 3.4f);
}

void drawMenus()
{
    if (g_state == STATE_MAIN_MENU || g_state == STATE_SOUND_MENU) {
        drawMenuBackground();
        panel(300, 120, 980, 560);
        drawTitle();
        if (g_state == STATE_SOUND_MENU) {
            drawTextCentered("VOLUME", 640, 376, 16, rgb(60, 40, 20), 3);
            int level = (int)roundf(getVolume() * 10.0f);
            for (int i = 0; i < 10; ++i) {
                float x = 540 + i * 20.0f;
                float h = 14 + i * 4.0f;
                Color c = (i < level && !isMuted()) ? rgb(40, 160, 80) : rgb(150, 140, 120);
                box(x, 305, x + 14, 305 + h, c);
            }
            if (isMuted()) drawTextCentered("MUTED", 640, 280, 12, rgb(196, 30, 44), 2.4f);
        }
    }
    else {
        // darkened game scene behind the menu
        setFade(0.4f);
        drawGame(true);
        setFade(1.0f);
        if (g_state == STATE_PAUSED) {
            panel(420, 170, 860, 500);
            drawTextCentered("PAUSED", 640, 432, 36, rgb(60, 40, 20), 6);
        }
        else if (g_state == STATE_WIN) {
            panel(380, 170, 900, 520);
            drawTextCentered("THE STATION IS SAFE!", 640, 440, 24, rgb(30, 120, 60), 4.5f);
            drawTextCentered("ALL THREE WAVES OF TRAFFIC DEFEATED", 640, 400, 12, rgb(60, 40, 20), 2.2f);
            Pose p; p.t = g_time; p.attack = fmodf(g_time, 1.0f);
            pushM(); translateM(460, 330 + fabsf(sinf(g_time * 5)) * 14); scaleM(0.6f, 0.6f); drawVendor(p); popM();
            pushM(); translateM(820, 330 + fabsf(cosf(g_time * 5)) * 14); scaleM(-0.6f, 0.6f); drawKid(p); popM();
        }
        else {
            panel(380, 170, 900, 520);
            drawTextCentered("THE STATION FELL!", 640, 440, 26, rgb(196, 30, 44), 4.5f);
            drawTextCentered("A VEHICLE REACHED THE METRO ENTRANCE", 640, 400, 12, rgb(60, 40, 20), 2.2f);
            Pose p; p.t = g_time; p.moving = false;
            pushM(); translateM(820, 328); scaleM(0.55f, 0.55f); drawTuktuk(p); popM();
            pushM(); translateM(460, 328); scaleM(0.55f, 0.55f); drawCar(p); popM();
        }
    }

    for (const Button& b : currentButtons()) drawButton(b);
}
