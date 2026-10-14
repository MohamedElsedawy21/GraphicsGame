#pragma once

// Everything a model's draw function needs to know to animate.
// The logic code (Persons 2 and 3) fills it in; the models (Person 4) read it.
struct AnimState {
    float time    = 0.0f;   // seconds since the character appeared (walk cycles, wheel spin)
    float attack  = 0.0f;   // 0 = idle, 0 -> 1 = progress of the current attack swing/throw
    float damage  = 0.0f;   // 0 = healthy, 1 = almost dead (used for the red tint)
    bool  moving  = true;   // false while stopped to fight
    bool  jumping = false;
    bool  ducking = false;
    float jumpOffset = 0.0f; // extra height while jumping, in pixels
};
