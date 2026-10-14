#pragma once

struct GLFWwindow;

// Keyboard and mouse. Mouse coordinates are in GAME pixels:
// (0, 0) is the bottom-left of the screen and y points up, just like drawing.
namespace Input {
    void init(GLFWwindow* window, int gameWidth, int gameHeight);
    void newFrame();   // call once per frame, BEFORE glfwPollEvents()

    // Keys use GLFW names: GLFW_KEY_W, GLFW_KEY_UP, GLFW_KEY_ESCAPE, ...
    bool keyDown(int key);      // held down right now
    bool keyPressed(int key);   // went down THIS frame (true once per press)

    float mouseX();
    float mouseY();
    bool  mouseDown();          // left button held
    bool  mouseClicked();       // left button went down THIS frame

    // True if the mouse is inside the rectangle (x, y = bottom-left corner)
    bool mouseIn(float x, float y, float w, float h);
}
