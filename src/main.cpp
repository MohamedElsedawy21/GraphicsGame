#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "engine/Renderer.h"
#include "engine/Input.h"
#include "engine/Audio.h"
#include "game/Game.h"

const int WINDOW_WIDTH  = 1280;
const int WINDOW_HEIGHT = 720;
const double LOGIC_STEP = 1.0 / 60.0;   // game logic always runs at 60 updates per second

int main() {
    // ---- 1. Window and OpenGL (same as Step 1) ----
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT,
                                          "Cairo Street Defense", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to load OpenGL\n";
        return -1;
    }
    std::cout << "OpenGL version: " << (const char*)glGetString(GL_VERSION) << "\n";

    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    glViewport(0, 0, fbWidth, fbHeight);

    // ---- 2. Engine parts ----
    if (!Renderer::init(WINDOW_WIDTH, WINDOW_HEIGHT)) {
        std::cerr << "Failed to initialize renderer\n";
        glfwTerminate();
        return -1;
    }
    Input::init(window, WINDOW_WIDTH, WINDOW_HEIGHT);
    Audio::init();   // if there is no sound device, the game still runs silently

    Game game;
    game.init();

    // ---- 3. The game loop with a fixed 60 Hz logic step ----
    double previousTime = glfwGetTime();
    double accumulator  = 0.0;

    while (!glfwWindowShouldClose(window) && !game.wantsToQuit()) {
        Input::newFrame();     // forget last frame's clicks and key presses...
        glfwPollEvents();      // ...then collect this frame's

        // How much real time passed since the last frame?
        double now = glfwGetTime();
        double frameTime = now - previousTime;
        previousTime = now;
        if (frameTime > 0.25) frameTime = 0.25;   // e.g. after dragging the window
        accumulator += frameTime;

        game.handleInput();

        // Run as many 1/60 s logic steps as the real time allows
        while (accumulator >= LOGIC_STEP) {
            game.update((float)LOGIC_STEP);
            accumulator -= LOGIC_STEP;
        }

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        Renderer::beginFrame();
        game.draw();

        glfwSwapBuffers(window);
    }

    // ---- 4. Clean up ----
    Audio::shutdown();
    Renderer::shutdown();
    glfwTerminate();
    return 0;
}
