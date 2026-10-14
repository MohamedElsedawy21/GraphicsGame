#include "engine/Renderer.h"
#include <glad/glad.h>
#include <iostream>
#include <cmath>

namespace {
    // ===== Section A: shaders =====
    const char* VERTEX_SHADER_SRC = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        uniform vec2 uScreenSize;

        void main() {
            vec2 ndc = (aPos / uScreenSize) * 2.0 - 1.0;
            gl_Position = vec4(ndc, 0.0, 1.0);
        }
    )";

    const char* FRAGMENT_SHADER_SRC = R"(
        #version 330 core
        out vec4 FragColor;
        uniform vec4  uColor;
        uniform float uDamage;
        uniform int   uGrayscale;

        void main() {
            vec3 c = uColor.rgb;
            if (uGrayscale == 1) {
                float gray = dot(c, vec3(0.299, 0.587, 0.114));
                c = vec3(gray);
            }
            c = mix(c, vec3(1.0, 0.0, 0.0), uDamage);
            FragColor = vec4(c, uColor.a);
        }
    )";

    // ===== Section B: saved state =====
    GLuint shaderProgram = 0;
    GLuint vao = 0;
    GLuint vbo = 0;

    GLint locScreenSize = -1;
    GLint locColor      = -1;
    GLint locDamage     = -1;
    GLint locGrayscale  = -1;

    const int   MAX_VERTICES    = 8192;
    const int   CIRCLE_SEGMENTS = 40;
    const float PI              = 3.14159265f;

    float outlineThickness = 0.0f;
    Color outlineColor {0.0f, 0.0f, 0.0f};

    // ===== Section C: the transform =====
    struct Transform {
        float a = 1, b = 0;   // where the x-axis points
        float c = 0, d = 1;   // where the y-axis points
        float tx = 0, ty = 0; // where the origin is

        Vec2 apply(Vec2 p) const {
            return { a * p.x + c * p.y + tx,
                     b * p.x + d * p.y + ty };
        }
    };

    std::vector<Transform> transformStack(1);

    // ===== Section D: helper functions =====
    GLuint compileShader(GLenum type, const char* source) {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &source, nullptr);
        glCompileShader(shader);

        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            char log[512];
            glGetShaderInfoLog(shader, 512, nullptr, log);
            std::cerr << "Shader compile error:\n" << log << "\n";
            glDeleteShader(shader);
            return 0;
        }
        return shader;
    }

    void drawVertices(const std::vector<Vec2>& points, GLenum mode, const Color& color) {
        int count = (int)points.size();
        if (count == 0) return;
        if (count > MAX_VERTICES) {
            std::cerr << "drawVertices: too many vertices (" << count << ")\n";
            return;
        }

        static float buffer[MAX_VERTICES * 2];
        const Transform& t = transformStack.back();
        for (int i = 0; i < count; i++) {
            Vec2 p = t.apply(points[i]);
            buffer[i * 2]     = p.x;
            buffer[i * 2 + 1] = p.y;
        }

        glUniform4f(locColor, color.r, color.g, color.b, color.a);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, count * 2 * sizeof(float), buffer);
        glDrawArrays(mode, 0, count);
    }

    void lineQuad(Vec2 p1, Vec2 p2, float thickness, bool extendEnds, const Color& color) {
        float dx = p2.x - p1.x;
        float dy = p2.y - p1.y;
        float len = std::sqrt(dx * dx + dy * dy);
        if (len == 0.0f) return;

        float ux = dx / len;
        float uy = dy / len;
        float half = thickness / 2.0f;

        if (extendEnds) {
            p1 = { p1.x - ux * half, p1.y - uy * half };
            p2 = { p2.x + ux * half, p2.y + uy * half };
        }

        float nx = -uy * half;
        float ny =  ux * half;

        drawVertices({
            { p1.x + nx, p1.y + ny },
            { p1.x - nx, p1.y - ny },
            { p2.x - nx, p2.y - ny },
            { p2.x + nx, p2.y + ny }
        }, GL_TRIANGLE_FAN, color);
    }

    void fillShape(const std::vector<Vec2>& points, const Color& color) {
        drawVertices(points, GL_TRIANGLE_FAN, color);

        if (outlineThickness > 0.0f) {
            for (size_t i = 0; i < points.size(); i++) {
                Vec2 p1 = points[i];
                Vec2 p2 = points[(i + 1) % points.size()];
                lineQuad(p1, p2, outlineThickness, true, outlineColor);
            }
        }
    }

    std::vector<Vec2> ellipsePoints(float cx, float cy, float rx, float ry) {
        std::vector<Vec2> pts;
        for (int i = 0; i < CIRCLE_SEGMENTS; i++) {
            float angle = 2.0f * PI * i / CIRCLE_SEGMENTS;
            pts.push_back({ cx + rx * std::cos(angle), cy + ry * std::sin(angle) });
        }
        return pts;
    }
}

// ===== Section E: setup =====
bool Renderer::init(int screenWidth, int screenHeight) {
    GLuint vs = compileShader(GL_VERTEX_SHADER, VERTEX_SHADER_SRC);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, FRAGMENT_SHADER_SRC);
    if (!vs || !fs) return false;

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vs);
    glAttachShader(shaderProgram, fs);
    glLinkProgram(shaderProgram);
    glDeleteShader(vs);
    glDeleteShader(fs);

    GLint success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char log[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, log);
        std::cerr << "Shader link error:\n" << log << "\n";
        return false;
    }

    locScreenSize = glGetUniformLocation(shaderProgram, "uScreenSize");
    locColor      = glGetUniformLocation(shaderProgram, "uColor");
    locDamage     = glGetUniformLocation(shaderProgram, "uDamage");
    locGrayscale  = glGetUniformLocation(shaderProgram, "uGrayscale");

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, MAX_VERTICES * 2 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(shaderProgram);
    glUniform2f(locScreenSize, (float)screenWidth, (float)screenHeight);
    setDamage(0.0f);
    setGrayscale(false);
    return true;
}

void Renderer::shutdown() {
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(shaderProgram);
}

void Renderer::beginFrame() {
    if (transformStack.size() != 1) {
        std::cerr << "Warning: pushTransform/popTransform not balanced last frame\n";
    }
    transformStack.assign(1, Transform{});
    setDamage(0.0f);
    setGrayscale(false);
    outlineThickness = 0.0f;
}

// ===== Section F: effects =====
void Renderer::setDamage(float amount) {
    glUniform1f(locDamage, amount);
}

void Renderer::setGrayscale(bool on) {
    glUniform1i(locGrayscale, on ? 1 : 0);
}

void Renderer::setOutline(float thickness, const Color& color) {
    outlineThickness = thickness;
    outlineColor = color;
}

// ===== Section G: shapes =====
void Renderer::drawRect(float x, float y, float w, float h, const Color& color) {
    fillShape({ {x, y}, {x + w, y}, {x + w, y + h}, {x, y + h} }, color);
}

void Renderer::drawRoundedRect(float x, float y, float w, float h, float radius,
                               const Color& color) {
    // Never let the corners be bigger than half the box
    radius = std::fmin(radius, std::fmin(w, h) / 2.0f);
    const int CORNER_STEPS = 6;

    // The 4 corner centers, going counter-clockwise from bottom-right
    Vec2 centers[4] = {
        { x + w - radius, y + radius     },
        { x + w - radius, y + h - radius },
        { x + radius,     y + h - radius },
        { x + radius,     y + radius     }
    };
    float startAngles[4] = { -90.0f, 0.0f, 90.0f, 180.0f };

    std::vector<Vec2> pts;
    for (int c = 0; c < 4; c++) {
        for (int i = 0; i <= CORNER_STEPS; i++) {
            float angle = (startAngles[c] + 90.0f * i / CORNER_STEPS) * PI / 180.0f;
            pts.push_back({ centers[c].x + radius * std::cos(angle),
                            centers[c].y + radius * std::sin(angle) });
        }
    }
    fillShape(pts, color);
}

void Renderer::drawCircle(float cx, float cy, float radius, const Color& color) {
    drawEllipse(cx, cy, radius, radius, color);
}

void Renderer::drawEllipse(float cx, float cy, float rx, float ry, const Color& color) {
    fillShape(ellipsePoints(cx, cy, rx, ry), color);
}

void Renderer::drawTriangle(float x1, float y1, float x2, float y2,
                            float x3, float y3, const Color& color) {
    fillShape({ {x1, y1}, {x2, y2}, {x3, y3} }, color);
}

void Renderer::drawTrapezoid(float x, float y, float bottomWidth, float topWidth,
                             float h, const Color& color) {
    float inset = (bottomWidth - topWidth) / 2.0f;
    fillShape({
        { x,                       y     },
        { x + bottomWidth,         y     },
        { x + bottomWidth - inset, y + h },
        { x + inset,               y + h }
    }, color);
}

void Renderer::drawPolygon(const std::vector<Vec2>& points, const Color& color) {
    fillShape(points, color);
}

void Renderer::drawLine(float x1, float y1, float x2, float y2,
                        float thickness, const Color& color) {
    lineQuad({x1, y1}, {x2, y2}, thickness, false, color);
}

void Renderer::drawTriangles(const std::vector<Vec2>& points, const Color& color) {
    drawVertices(points, GL_TRIANGLES, color);
}

// ===== Section H: transforms =====
void Renderer::pushTransform() {
    transformStack.push_back(transformStack.back());
}

void Renderer::popTransform() {
    if (transformStack.size() > 1) {
        transformStack.pop_back();
    } else {
        std::cerr << "Warning: popTransform called with nothing to pop\n";
    }
}

void Renderer::translate(float x, float y) {
    Transform& t = transformStack.back();
    t.tx += t.a * x + t.c * y;
    t.ty += t.b * x + t.d * y;
}

void Renderer::rotate(float degrees) {
    float rad = degrees * PI / 180.0f;
    float cs = std::cos(rad);
    float sn = std::sin(rad);

    Transform& t = transformStack.back();
    float a = t.a, b = t.b, c = t.c, d = t.d;
    t.a =  a * cs + c * sn;
    t.b =  b * cs + d * sn;
    t.c = -a * sn + c * cs;
    t.d = -b * sn + d * cs;
}

void Renderer::scale(float sx, float sy) {
    Transform& t = transformStack.back();
    t.a *= sx;  t.b *= sx;
    t.c *= sy;  t.d *= sy;
}
