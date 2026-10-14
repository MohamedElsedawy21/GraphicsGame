#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include "engine/Renderer.h"

const int WINDOW_WIDTH  = 1280;
const int WINDOW_HEIGHT = 720;

int main() {
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

    if (!Renderer::init(WINDOW_WIDTH, WINDOW_HEIGHT)) {
        std::cerr << "Failed to initialize renderer\n";
        glfwTerminate();
        return -1;
    }

    Color road    {0.35f, 0.35f, 0.38f};
    Color rust    {0.79f, 0.40f, 0.12f};
    Color yellow  {0.95f, 0.78f, 0.15f};
    Color dark    {0.15f, 0.15f, 0.17f};
    Color gray    {0.70f, 0.70f, 0.72f};
    Color white   {0.95f, 0.95f, 0.95f};
    Color outline {0.10f, 0.10f, 0.10f};

    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        float time = (float)glfwGetTime();

        glClearColor(0.53f, 0.75f, 0.92f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        Renderer::beginFrame();

        // ---- Test scene (temporary) ----
        Renderer::drawRect(0, 0, 1280, 150, road);
        Renderer::drawLine(0, 75, 1280, 75, 4, white);

        Renderer::setOutline(2.0f, outline);

        // Row of every shape
        Renderer::drawCircle(120, 300, 40, rust);
        Renderer::drawEllipse(260, 300, 50, 25, yellow);
        Renderer::drawTriangle(360, 260, 460, 260, 410, 340, rust);
        Renderer::drawTrapezoid(510, 260, 100, 60, 80, yellow);
        Renderer::drawPolygon({ {660, 260}, {760, 260}, {790, 300},
                                {730, 345}, {650, 305} }, rust);

        // Spinning wheel: rotate around its center
        Renderer::pushTransform();
        Renderer::translate(920, 300);
        Renderer::rotate(-time * 180.0f);
        Renderer::drawCircle(0, 0, 40, dark);
        Renderer::drawRect(-4, -34, 8, 68, gray);
        Renderer::drawRect(-34, -4, 68, 8, gray);
        Renderer::popTransform();

        // Swinging arm: rotate around the shoulder
        Renderer::pushTransform();
        Renderer::translate(1100, 360);
        Renderer::rotate(std::sin(time * 4.0f) * 45.0f);
        Renderer::drawRect(-5, -60, 10, 60, rust);
        Renderer::drawEllipse(0, -62, 14, 6, yellow);
        Renderer::popTransform();
        Renderer::drawCircle(1100, 360, 5, dark);

        Renderer::setOutline(0.0f, outline);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    Renderer::shutdown();
    glfwTerminate();
    return 0;
}