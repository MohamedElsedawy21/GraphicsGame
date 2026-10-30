// ============================================================================
//  renderer.h : shader + one global VBO/VAO/EBO holding every basic shape
//               (Lab 3 "Global VBO" idea: draw a range with glDrawArrays)
// ============================================================================
#pragma once
#include "common.h"

typedef glm::vec3 Color;
Color rgb(int r, int g, int b);                 // 0..255 -> 0..1
Color lerpColor(Color a, Color b, float t);

void initRenderer();
void cleanupRenderer();
void beginFrame(Color clear);

// ---------------------------------------------------------- matrix stack
// Every shape is drawn with  u_Transform = view * model * local
// (view maps world pixels to NDC using translate + scale, exactly like Lab 2)
void pushM();
void popM();
void translateM(float x, float y);
void rotateM(float degrees);
void scaleM(float sx, float sy);

// ---------------------------------------------------------- fragment state
void setDamage(float amount);   // mix(color, red, amount)   (u_Highlight)
void setHighlightColor(Color c);
void setGray(float amount);     // 0 = normal colours, 1 = grayscale (locked cards)
void setFade(float f);          // 1 = normal, 0 = black (cutscene fades)

// ---------------------------------------------------------- shapes
void rect(float cx, float cy, float w, float h, Color c);                 // centred
void rectRot(float cx, float cy, float w, float h, float deg, Color c);   // rotated
void box(float x0, float y0, float x1, float y1, Color c);                // corners
void shadeBox(float x0, float y0, float x1, float y1, Color c);           // top lighter, bottom darker
void circle(float cx, float cy, float r, Color c);
void ellipse(float cx, float cy, float rx, float ry, Color c);
void ellipseRot(float cx, float cy, float rx, float ry, float deg, Color c);
void ball(float cx, float cy, float r, Color c);                          // shaded circle
void semicircle(float cx, float cy, float rx, float ry, float deg, Color c); // upper half rotated
void tri(float x1, float y1, float x2, float y2, float x3, float y3, Color c);
void quad(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, Color c);
void line(float x1, float y1, float x2, float y2, float thick, Color c);
void roundLine(float x1, float y1, float x2, float y2, float thick, Color c);
void arc(float cx, float cy, float r, float a0, float a1, float thick, Color c);
void roundBox(float x0, float y0, float x1, float y1, float r, Color c);

// ---------------------------------------------------------- text (stroke font made of quads)
void  drawText(const std::string& s, float x, float y, float h, Color c, float thick = -1.0f);
void  drawTextCentered(const std::string& s, float cx, float y, float h, Color c, float thick = -1.0f);
float textWidth(const std::string& s, float h);

// ---------------------------------------------------------- collision helpers (Lab 3)
float triSign(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3);
bool  pointInTriangle(glm::vec2 pt, glm::vec2 a, glm::vec2 b, glm::vec2 c);
bool  pointInRect(float px, float py, float cx, float cy, float w, float h);  // 2 triangles
