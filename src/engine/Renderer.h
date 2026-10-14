#pragma once

struct Color {
    float r, g, b;
    float a = 1.0f;
};

namespace Renderer {
    bool init(int screenWidth, int screenHeight);
    void shutdown();

    void setDamage(float amount);
    void setGrayscale(bool on);

    void drawRect(float x, float y, float w, float h, const Color& color);
}