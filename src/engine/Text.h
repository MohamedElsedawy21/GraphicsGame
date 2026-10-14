#pragma once
#include <string>
#include "engine/Renderer.h"

// Text drawing. (x, y) is the BOTTOM-LEFT of the capital letters (the baseline).
// size = height of a capital letter in pixels (e.g. 14 for small UI text, 48 for titles).
namespace Text {
    void  draw(const std::string& text, float x, float y, float size, const Color& color);
    void  drawCentered(const std::string& text, float centerX, float y, float size, const Color& color);
    void  drawShadowed(const std::string& text, float x, float y, float size,
                       const Color& color, const Color& shadow);
    float width(const std::string& text, float size);
}
