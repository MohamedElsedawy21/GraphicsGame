// ============================================================================
//  METRO DEFENDERS - "The Egyptian Street"
//  common.h : includes + shared constants/globals used by every file
// ============================================================================
#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <cstdlib>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Window size (all drawing is done in "world pixels": x 0..1280, y 0..720, y UP)
const int SCREEN_W = 1280;
const int SCREEN_H = 720;
const float PI = 3.14159265f;

// ---------------------------------------------------------------- game states
enum GameState {
    STATE_MAIN_MENU,
    STATE_SOUND_MENU,
    STATE_CUTSCENE,
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_WIN,
    STATE_LOSE
};

extern GameState   g_state;
extern GLFWwindow* g_window;
extern float       g_mouseX;     // cursor in world pixels (y up)
extern float       g_mouseY;
extern float       g_time;       // seconds since start (glfwGetTime)

// Small helper used by menus/cutscene to switch state
void goToState(GameState s);
