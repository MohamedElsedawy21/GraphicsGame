#pragma once
#include <vector>

struct Color {
    float r, g, b;
    float a = 1.0f;
};

struct Vec2 {
    float x, y;
};

namespace Renderer {
    // ---- Setup ----
    bool init(int screenWidth, int screenHeight);
    void shutdown();
    void beginFrame();

    // ---- Effects (stay on until changed) ----
    void setDamage(float amount);
    void setGrayscale(bool on);
    void setOutline(float thickness, const Color& color);

    // ---- Filled shapes ----
    void drawRect(float x, float y, float w, float h, const Color& color);
    void drawCircle(float cx, float cy, float radius, const Color& color);
    void drawEllipse(float cx, float cy, float rx, float ry, const Color& color);
    void drawTriangle(float x1, float y1, float x2, float y2,
                      float x3, float y3, const Color& color);
    void drawTrapezoid(float x, float y, float bottomWidth, float topWidth,
                       float h, const Color& color);
    void drawPolygon(const std::vector<Vec2>& points, const Color& color);

    // ---- Lines (never outlined) ----
    void drawLine(float x1, float y1, float x2, float y2,
                  float thickness, const Color& color);

    // ---- Transforms ----
    void pushTransform();
    void popTransform();
    void translate(float x, float y);
    void rotate(float degrees);
    void scale(float sx, float sy);
}