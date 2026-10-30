// ============================================================================
//  METRO DEFENDERS - "The Egyptian Street"
//  Pedestrians defend their metro station from tuk-tuks, microbuses and taxis.
//
//  main.cpp : window, input callbacks (Lab 3), game loop with deltaTime (Lab 2)
// ============================================================================
#include "common.h"
#include "renderer.h"
#include "game.h"
#include "menus.h"
#include "cutscene.h"
#include "audio.h"
#include <ctime>

GameState   g_state = STATE_MAIN_MENU;
GLFWwindow* g_window = nullptr;
float       g_mouseX = 0.0f;
float       g_mouseY = 0.0f;
float       g_time = 0.0f;

void goToState(GameState s)
{
    g_state = s;
    if (s == STATE_CUTSCENE) startCutscene();
}

// ------------------------------------------------------------------ callbacks
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS) return;      // once per press (no spamming)

    switch (g_state) {
    case STATE_PLAYING:
        if (key == GLFW_KEY_ESCAPE) { goToState(STATE_PAUSED); playSfx("select"); }
        else gameKey(key);
        break;
    case STATE_PAUSED:
        if (key == GLFW_KEY_ESCAPE) goToState(STATE_PLAYING);
        break;
    case STATE_CUTSCENE:
        if (key == GLFW_KEY_ENTER || key == GLFW_KEY_SPACE || key == GLFW_KEY_ESCAPE) skipCutscene();
        break;
    case STATE_SOUND_MENU:
        if (key == GLFW_KEY_ESCAPE) goToState(STATE_MAIN_MENU);
        break;
    default:
        break;
    }
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (action != GLFW_PRESS) return;
    if (g_state == STATE_PLAYING) gameMouseClick(button);
    else if (g_state != STATE_CUTSCENE) menuMouseClick(button);
}

void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    // window pixels (y down) -> world pixels (y up)
    int w, h;
    glfwGetWindowSize(window, &w, &h);
    if (w <= 0 || h <= 0) return;
    g_mouseX = (float)xpos * SCREEN_W / w;
    g_mouseY = SCREEN_H - (float)ypos * SCREEN_H / h;
}

// ------------------------------------------------------------------ main
int main()
{
    srand((unsigned)time(nullptr));

    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    g_window = glfwCreateWindow(SCREEN_W, SCREEN_H, "Metro Defenders - The Egyptian Street", nullptr, nullptr);
    if (!g_window) {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(g_window);
    glfwSwapInterval(1);

    glfwSetKeyCallback(g_window, keyCallback);
    glfwSetMouseButtonCallback(g_window, mouseButtonCallback);
    glfwSetCursorPosCallback(g_window, cursorPositionCallback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to init GLAD\n";
        return -1;
    }

    glViewport(0, 0, SCREEN_W, SCREEN_H);

    initRenderer();
    audioInit();
    resetGame();

    float lastFrame = (float)glfwGetTime();

    while (!glfwWindowShouldClose(g_window))
    {
        float currentFrame = (float)glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        if (deltaTime > 0.05f) deltaTime = 0.05f;      // avoid huge jumps (window dragged, etc.)
        g_time = currentFrame;

        // ---- update
        switch (g_state) {
        case STATE_PLAYING:  updateGame(deltaTime); break;
        case STATE_CUTSCENE: updateCutscene(deltaTime); break;
        default:             updateMenus(deltaTime); break;      // PAUSED / WIN / LOSE freeze the game
        }

        // ---- draw
        beginFrame(Color(0.1f, 0.1f, 0.15f));
        switch (g_state) {
        case STATE_PLAYING:  drawGame(true); break;
        case STATE_CUTSCENE: drawCutscene(); break;
        default:             drawMenus(); break;
        }

        glfwSwapBuffers(g_window);
        glfwPollEvents();
    }

    audioShutdown();
    cleanupRenderer();
    glfwTerminate();
    return 0;
}
