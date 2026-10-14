#pragma once
#include <string>

// A clickable menu button. (x, y) = bottom-left corner.
class Button {
public:
    Button(float x, float y, float w, float h, const std::string& label);

    bool isHovered() const;
    bool wasClicked() const;   // true on the frame it is clicked (also plays the click sound)
    void draw() const;

    float x, y, w, h;
    std::string label;
};
