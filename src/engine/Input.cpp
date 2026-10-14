#include "engine/Input.h"
#define GLFW_INCLUDE_NONE   // we only need GLFW key names here, not OpenGL
#include <GLFW/glfw3.h>

namespace {
    GLFWwindow* window = nullptr;
    int gameW = 1280;
    int gameH = 720;

    bool keysPressedThisFrame[GLFW_KEY_LAST + 1] = {};
    bool clickedThisFrame = false;

    // GLFW calls this every time any key changes state
    void keyCallback(GLFWwindow*, int key, int, int action, int) {
        if (key >= 0 && key <= GLFW_KEY_LAST && action == GLFW_PRESS) {
            keysPressedThisFrame[key] = true;
        }
    }

    // GLFW calls this every time a mouse button changes state
    void mouseButtonCallback(GLFWwindow*, int button, int action, int) {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            clickedThisFrame = true;
        }
    }
}

void Input::init(GLFWwindow* w, int gameWidth, int gameHeight) {
    window = w;
    gameW = gameWidth;
    gameH = gameHeight;
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
}

void Input::newFrame() {
    for (bool& k : keysPressedThisFrame) k = false;
    clickedThisFrame = false;
}

bool Input::keyDown(int key) {
    return glfwGetKey(window, key) == GLFW_PRESS;
}

bool Input::keyPressed(int key) {
    if (key < 0 || key > GLFW_KEY_LAST) return false;
    return keysPressedThisFrame[key];
}

float Input::mouseX() {
    double cx, cy;
    int winW, winH;
    glfwGetCursorPos(window, &cx, &cy);
    glfwGetWindowSize(window, &winW, &winH);
    return (float)(cx * gameW / winW);
}

float Input::mouseY() {
    double cx, cy;
    int winW, winH;
    glfwGetCursorPos(window, &cx, &cy);
    glfwGetWindowSize(window, &winW, &winH);
    // GLFW measures y from the TOP; our game measures from the BOTTOM
    return (float)(gameH - cy * gameH / winH);
}

bool Input::mouseDown() {
    return glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
}

bool Input::mouseClicked() {
    return clickedThisFrame;
}

bool Input::mouseIn(float x, float y, float w, float h) {
    float mx = mouseX();
    float my = mouseY();
    return mx >= x && mx <= x + w && my >= y && my <= y + h;
}
