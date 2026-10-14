#include "engine/Text.h"
#include <vector>

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"   // stb has functions we don't use
#endif
#include "stb_easy_font.h"   // a tiny public-domain font made of rectangles
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

namespace {
    // stb_easy_font draws capital letters 7 units tall
    const float FONT_CAP_HEIGHT = 7.0f;

    // One vertex as stb_easy_font writes it: x, y, z, then 4 color bytes
    struct StbVertex {
        float x, y, z;
        unsigned char color[4];
    };
}

void Text::draw(const std::string& text, float x, float y, float size, const Color& color) {
    if (text.empty()) return;

    // 1. Ask stb_easy_font for the letter shapes (as quads, 4 corners each)
    static StbVertex buffer[6000];
    int quads = stb_easy_font_print(0, 0, const_cast<char*>(text.c_str()), nullptr,
                                    buffer, sizeof(buffer));

    // 2. Turn every quad (4 corners) into 2 triangles (6 corners)
    std::vector<Vec2> triangles;
    triangles.reserve(quads * 6);
    for (int q = 0; q < quads; q++) {
        StbVertex* v = &buffer[q * 4];
        triangles.push_back({v[0].x, v[0].y});
        triangles.push_back({v[1].x, v[1].y});
        triangles.push_back({v[2].x, v[2].y});
        triangles.push_back({v[0].x, v[0].y});
        triangles.push_back({v[2].x, v[2].y});
        triangles.push_back({v[3].x, v[3].y});
    }

    // 3. stb_easy_font's y points DOWN; ours points UP. Flip with a negative scale.
    float s = size / FONT_CAP_HEIGHT;
    Renderer::pushTransform();
    Renderer::translate(x, y + size);
    Renderer::scale(s, -s);
    Renderer::drawTriangles(triangles, color);
    Renderer::popTransform();
}

void Text::drawCentered(const std::string& text, float centerX, float y, float size,
                        const Color& color) {
    draw(text, centerX - width(text, size) / 2.0f, y, size, color);
}

void Text::drawShadowed(const std::string& text, float x, float y, float size,
                        const Color& color, const Color& shadow) {
    float offset = size / 10.0f + 1.0f;
    draw(text, x + offset, y - offset, size, shadow);
    draw(text, x, y, size, color);
}

float Text::width(const std::string& text, float size) {
    if (text.empty()) return 0.0f;
    // stb adds 1-2 units of empty space after the last letter; remove it
    float units = (float)stb_easy_font_width(const_cast<char*>(text.c_str())) - 2.0f;
    return units * (size / FONT_CAP_HEIGHT);
}
