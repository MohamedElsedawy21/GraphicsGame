#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "engine/Renderer.h"                                   // NEW

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

    if (!Renderer::init(WINDOW_WIDTH, WINDOW_HEIGHT)) {       // NEW
        std::cerr << "Failed to initialize renderer\n";       // NEW
        glfwTerminate();                                       // NEW
        return -1;                                             // NEW
    }                                                          // NEW

    Color road   {0.35f, 0.35f, 0.38f};                        // NEW
    Color rust   {0.79f, 0.40f, 0.12f};                        // NEW

    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        glClearColor(0.53f, 0.75f, 0.92f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // ---- Test drawing (temporary) ----                  // NEW
        Renderer::drawRect(0, 0, 1280, 150, road);             // NEW

        Renderer::drawRect(200, 150, 150, 80, rust);           // NEW

        Renderer::setDamage(0.5f);                             // NEW
        Renderer::drawRect(450, 150, 150, 80, rust);           // NEW
        Renderer::setDamage(0.0f);                             // NEW

        Renderer::setGrayscale(true);                          // NEW
        Renderer::drawRect(700, 150, 150, 80, rust);           // NEW
        Renderer::setGrayscale(false);                         // NEW

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    Renderer::shutdown();                                      // NEW
    glfwTerminate();
    return 0;
}