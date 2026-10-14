#include "engine/Renderer.h"
#include <glad/glad.h>
#include <iostream>

namespace {
    // ===== Section A: the shaders =====
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

    // ===== Section B: saved GPU handles =====
    GLuint shaderProgram = 0;
    GLuint vao = 0;
    GLuint vbo = 0;

    GLint locScreenSize = -1;
    GLint locColor      = -1;
    GLint locDamage     = -1;
    GLint locGrayscale  = -1;

    const int MAX_VERTICES = 1024;

    // ===== Section C: helper functions =====
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

    void drawVertices(const float* vertices, int count, GLenum mode, const Color& color) {
        if (count > MAX_VERTICES) {
            std::cerr << "drawVertices: too many vertices (" << count << ")\n";
            return;
        }
        glUniform4f(locColor, color.r, color.g, color.b, color.a);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, count * 2 * sizeof(float), vertices);
        glDrawArrays(mode, 0, count);
    }
}

// ===== Section D: setup and cleanup =====
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

// ===== Section E: public drawing functions =====
void Renderer::setDamage(float amount) {
    glUniform1f(locDamage, amount);
}

void Renderer::setGrayscale(bool on) {
    glUniform1i(locGrayscale, on ? 1 : 0);
}

void Renderer::drawRect(float x, float y, float w, float h, const Color& color) {
    float vertices[] = {
        x,     y,
        x + w, y,
        x + w, y + h,
        x,     y + h
    };
    drawVertices(vertices, 4, GL_TRIANGLE_FAN, color);
}