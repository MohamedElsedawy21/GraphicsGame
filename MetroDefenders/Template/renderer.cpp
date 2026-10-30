// ============================================================================
//  renderer.cpp
// ============================================================================
#include "renderer.h"

// --------------------------------------------------------------- shaders
// Vertex shader: same as Lab 2 (position + colour + u_Transform)
const char* vertexShaderSource = R"(
#version 430 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 vColor;

uniform mat4 u_Transform;

void main()
{
    gl_Position = u_Transform * vec4(aPos, 1.0);
    vColor = aColor;
}
)";

// Fragment shader: Lab 2 tint  +  Lab 3 mix() highlight
//   u_Tint      -> colour of the shape (vertex colour * tint)
//   u_Highlight -> how much we mix towards u_HighlightColor (red = damage)
//   u_Gray      -> how much we mix towards the gray value (locked cards)
//   u_Fade      -> darken everything (cutscene fade in/out)
const char* fragmentShaderSource = R"(
#version 430 core

in vec3 vColor;
out vec4 FragColor;

uniform vec4  u_Tint;
uniform float u_Highlight;
uniform vec3  u_HighlightColor;
uniform float u_Gray;
uniform float u_Fade;

void main()
{
    vec3 c = vColor * u_Tint.rgb;
    c = mix(c, u_HighlightColor, u_Highlight);
    float g = (c.r + c.g + c.b) / 3.0;
    c = mix(c, vec3(g, g, g), u_Gray);
    FragColor = vec4(c * u_Fade, 1.0);
}
)";

static GLuint compileShader(GLenum type, const char* source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compile error ("
            << (type == GL_VERTEX_SHADER ? "vertex" : "fragment")
            << "):\n" << infoLog << std::endl;
    }
    return shader;
}

// --------------------------------------------------------------- GPU objects
static GLuint VAO, VBO, EBO;
static GLuint shaderProgram;
static GLint locTransform, locTint, locHighlight, locHighlightColor, locGray, locFade;

// where each shape lives inside the single VBO  (first vertex, vertex count)
const int SEGMENTS = 40;                 // circle smoothness
static int quadStart = 0;                // 4 vertices, drawn with the EBO (6 indices)
static int shadeStart = 0, shadeCount = 4;
static int circleStart = 0, circleCount = 0;
static int ballStart = 0, ballCount = 0;
static int halfStart = 0, halfCount = 0;
static int triStart = 0;                 // unit right triangle (0,0) (1,0) (0,1)

// view matrix: world pixels -> NDC     ( translate(-1,-1) * scale(2/W, 2/H) )
static glm::mat4 g_view;
static std::vector<glm::mat4> g_stack;
static float g_fadeValue = 1.0f;

Color rgb(int r, int g, int b) { return Color(r / 255.0f, g / 255.0f, b / 255.0f); }
Color lerpColor(Color a, Color b, float t) { return a + (b - a) * t; }

static void addVertex(std::vector<float>& v, float x, float y, float c)
{
    v.insert(v.end(), { x, y, 0.0f, c, c, c });
}

static void buildFont();

void initRenderer()
{
    // ---- build shader program (Lab 1 / Lab 2)
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    int success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Shader link error:\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // ---- every basic shape goes into ONE vector -> ONE VBO (Lab 3 global VBO)
    std::vector<float> vertices;

    // unit quad (-0.5..0.5) drawn with EBO
    quadStart = 0;
    addVertex(vertices, -0.5f, -0.5f, 1.0f);   // 0
    addVertex(vertices,  0.5f, -0.5f, 1.0f);   // 1
    addVertex(vertices,  0.5f,  0.5f, 1.0f);   // 2
    addVertex(vertices, -0.5f,  0.5f, 1.0f);   // 3

    // shaded quad: top vertices bright, bottom darker -> gives volume (colour interpolation)
    shadeStart = 4;
    addVertex(vertices, -0.5f, -0.5f, 0.72f);
    addVertex(vertices,  0.5f, -0.5f, 0.72f);
    addVertex(vertices,  0.5f,  0.5f, 1.0f);
    addVertex(vertices, -0.5f,  0.5f, 1.0f);

    // unit circle (triangle fan: centre + SEGMENTS+1 rim points)
    circleStart = vertices.size() / 6;
    addVertex(vertices, 0.0f, 0.0f, 1.0f);
    for (int i = 0; i <= SEGMENTS; ++i) {
        float angle = 2.0f * PI * (float)i / (float)SEGMENTS;
        addVertex(vertices, cosf(angle), sinf(angle), 1.0f);
    }
    circleCount = SEGMENTS + 2;

    // shaded "ball": fan centre moved to the upper-left & brighter, rim darker
    ballStart = vertices.size() / 6;
    addVertex(vertices, -0.3f, 0.3f, 1.15f);
    for (int i = 0; i <= SEGMENTS; ++i) {
        float angle = 2.0f * PI * (float)i / (float)SEGMENTS;
        addVertex(vertices, cosf(angle), sinf(angle), 0.7f);
    }
    ballCount = SEGMENTS + 2;

    // upper half circle
    halfStart = vertices.size() / 6;
    addVertex(vertices, 0.0f, 0.0f, 1.0f);
    for (int i = 0; i <= SEGMENTS / 2; ++i) {
        float angle = PI * (float)i / (float)(SEGMENTS / 2);
        addVertex(vertices, cosf(angle), sinf(angle), 1.0f);
    }
    halfCount = SEGMENTS / 2 + 2;

    // unit right triangle (any triangle is made from it with a matrix)
    triStart = vertices.size() / 6;
    addVertex(vertices, 0.0f, 0.0f, 1.0f);
    addVertex(vertices, 1.0f, 0.0f, 1.0f);
    addVertex(vertices, 0.0f, 1.0f, 1.0f);

    GLuint indices[] = {
        0, 1, 2,
        0, 2, 3
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    // (the EBO stays recorded inside the VAO)

    locTransform = glGetUniformLocation(shaderProgram, "u_Transform");
    locTint = glGetUniformLocation(shaderProgram, "u_Tint");
    locHighlight = glGetUniformLocation(shaderProgram, "u_Highlight");
    locHighlightColor = glGetUniformLocation(shaderProgram, "u_HighlightColor");
    locGray = glGetUniformLocation(shaderProgram, "u_Gray");
    locFade = glGetUniformLocation(shaderProgram, "u_Fade");

    // view: world pixels (0..W, 0..H)  ->  NDC (-1..1)
    g_view = glm::mat4(1.0f);
    g_view = glm::translate(g_view, glm::vec3(-1.0f, -1.0f, 0.0f));
    g_view = glm::scale(g_view, glm::vec3(2.0f / SCREEN_W, 2.0f / SCREEN_H, 1.0f));

    g_stack.clear();
    g_stack.push_back(glm::mat4(1.0f));

    buildFont();
}

void cleanupRenderer()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);
}

void beginFrame(Color clear)
{
    glClearColor(clear.r, clear.g, clear.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);

    g_stack.clear();
    g_stack.push_back(glm::mat4(1.0f));
    setDamage(0.0f);
    setHighlightColor(Color(0.95f, 0.08f, 0.05f));
    setGray(0.0f);
    setFade(1.0f);
}

// --------------------------------------------------------------- matrix stack
void pushM() { g_stack.push_back(g_stack.back()); }
void popM() { if (g_stack.size() > 1) g_stack.pop_back(); }
void translateM(float x, float y) { g_stack.back() = glm::translate(g_stack.back(), glm::vec3(x, y, 0.0f)); }
void rotateM(float degrees) { g_stack.back() = glm::rotate(g_stack.back(), glm::radians(degrees), glm::vec3(0.0f, 0.0f, 1.0f)); }
void scaleM(float sx, float sy) { g_stack.back() = glm::scale(g_stack.back(), glm::vec3(sx, sy, 1.0f)); }

// --------------------------------------------------------------- fragment state
void setDamage(float amount) { glUniform1f(locHighlight, glm::clamp(amount, 0.0f, 1.0f)); }
void setHighlightColor(Color c) { glUniform3f(locHighlightColor, c.r, c.g, c.b); }
void setGray(float amount) { glUniform1f(locGray, amount); }
void setFade(float f) { g_fadeValue = f; glUniform1f(locFade, f); }

// --------------------------------------------------------------- core draw
enum ShapeKind { SHAPE_QUAD, SHAPE_SHADE, SHAPE_CIRCLE, SHAPE_BALL, SHAPE_HALF, SHAPE_TRI };

static void drawShape(ShapeKind kind, const glm::mat4& local, Color c)
{
    glm::mat4 transform = g_view * g_stack.back() * local;
    glUniformMatrix4fv(locTransform, 1, GL_FALSE, glm::value_ptr(transform));
    glUniform4f(locTint, c.r, c.g, c.b, 1.0f);

    switch (kind) {
    case SHAPE_QUAD:   glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); break;
    case SHAPE_SHADE:  glDrawArrays(GL_TRIANGLE_FAN, shadeStart, shadeCount); break;
    case SHAPE_CIRCLE: glDrawArrays(GL_TRIANGLE_FAN, circleStart, circleCount); break;
    case SHAPE_BALL:   glDrawArrays(GL_TRIANGLE_FAN, ballStart, ballCount); break;
    case SHAPE_HALF:   glDrawArrays(GL_TRIANGLE_FAN, halfStart, halfCount); break;
    case SHAPE_TRI:    glDrawArrays(GL_TRIANGLES, triStart, 3); break;
    }
}

static glm::mat4 TRS(float cx, float cy, float deg, float sx, float sy)
{
    glm::mat4 m = glm::mat4(1.0f);
    m = glm::translate(m, glm::vec3(cx, cy, 0.0f));
    if (deg != 0.0f) m = glm::rotate(m, glm::radians(deg), glm::vec3(0.0f, 0.0f, 1.0f));
    m = glm::scale(m, glm::vec3(sx, sy, 1.0f));
    return m;
}

// --------------------------------------------------------------- shapes
void rect(float cx, float cy, float w, float h, Color c) { drawShape(SHAPE_QUAD, TRS(cx, cy, 0, w, h), c); }
void rectRot(float cx, float cy, float w, float h, float deg, Color c) { drawShape(SHAPE_QUAD, TRS(cx, cy, deg, w, h), c); }
void box(float x0, float y0, float x1, float y1, Color c) { rect((x0 + x1) * 0.5f, (y0 + y1) * 0.5f, fabsf(x1 - x0), fabsf(y1 - y0), c); }
void shadeBox(float x0, float y0, float x1, float y1, Color c)
{
    drawShape(SHAPE_SHADE, TRS((x0 + x1) * 0.5f, (y0 + y1) * 0.5f, 0, fabsf(x1 - x0), fabsf(y1 - y0)), c);
}
void circle(float cx, float cy, float r, Color c) { drawShape(SHAPE_CIRCLE, TRS(cx, cy, 0, r, r), c); }
void ellipse(float cx, float cy, float rx, float ry, Color c) { drawShape(SHAPE_CIRCLE, TRS(cx, cy, 0, rx, ry), c); }
void ellipseRot(float cx, float cy, float rx, float ry, float deg, Color c) { drawShape(SHAPE_CIRCLE, TRS(cx, cy, deg, rx, ry), c); }
void ball(float cx, float cy, float r, Color c) { drawShape(SHAPE_BALL, TRS(cx, cy, 0, r, r), c); }
void semicircle(float cx, float cy, float rx, float ry, float deg, Color c) { drawShape(SHAPE_HALF, TRS(cx, cy, deg, rx, ry), c); }

// Any triangle: a matrix whose columns send (1,0)->p2-p1, (0,1)->p3-p1 and moves origin to p1
void tri(float x1, float y1, float x2, float y2, float x3, float y3, Color c)
{
    glm::mat4 m(1.0f);
    m[0] = glm::vec4(x2 - x1, y2 - y1, 0.0f, 0.0f);
    m[1] = glm::vec4(x3 - x1, y3 - y1, 0.0f, 0.0f);
    m[3] = glm::vec4(x1, y1, 0.0f, 1.0f);
    drawShape(SHAPE_TRI, m, c);
}

void quad(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, Color c)
{
    tri(x1, y1, x2, y2, x3, y3, c);
    tri(x1, y1, x3, y3, x4, y4, c);
}

void line(float x1, float y1, float x2, float y2, float thick, Color c)
{
    float dx = x2 - x1, dy = y2 - y1;
    float len = sqrtf(dx * dx + dy * dy);
    float deg = glm::degrees(atan2f(dy, dx));
    rectRot((x1 + x2) * 0.5f, (y1 + y2) * 0.5f, len, thick, deg, c);
}

void roundLine(float x1, float y1, float x2, float y2, float thick, Color c)
{
    line(x1, y1, x2, y2, thick, c);
    circle(x1, y1, thick * 0.5f, c);
    circle(x2, y2, thick * 0.5f, c);
}

void arc(float cx, float cy, float r, float a0, float a1, float thick, Color c)
{
    const int steps = 8;
    for (int i = 0; i < steps; ++i) {
        float t0 = glm::radians(a0 + (a1 - a0) * i / steps);
        float t1 = glm::radians(a0 + (a1 - a0) * (i + 1) / steps);
        roundLine(cx + r * cosf(t0), cy + r * sinf(t0), cx + r * cosf(t1), cy + r * sinf(t1), thick, c);
    }
}

void roundBox(float x0, float y0, float x1, float y1, float r, Color c)
{
    box(x0 + r, y0, x1 - r, y1, c);
    box(x0, y0 + r, x1, y1 - r, c);
    circle(x0 + r, y0 + r, r, c);
    circle(x1 - r, y0 + r, r, c);
    circle(x0 + r, y1 - r, r, c);
    circle(x1 - r, y1 - r, r, c);
}

// --------------------------------------------------------------- collision (Lab 3)
float triSign(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3)
{
    return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}

bool pointInTriangle(glm::vec2 pt, glm::vec2 a, glm::vec2 b, glm::vec2 c)
{
    float d1 = triSign(pt, a, b);
    float d2 = triSign(pt, b, c);
    float d3 = triSign(pt, c, a);

    bool hasNeg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    bool hasPos = (d1 > 0) || (d2 > 0) || (d3 > 0);

    return !(hasNeg && hasPos);
}

// a rectangle is 2 triangles -> reuse pointInTriangle
bool pointInRect(float px, float py, float cx, float cy, float w, float h)
{
    glm::vec2 p(px, py);
    glm::vec2 a(cx - w / 2, cy - h / 2), b(cx + w / 2, cy - h / 2);
    glm::vec2 c(cx + w / 2, cy + h / 2), d(cx - w / 2, cy + h / 2);
    return pointInTriangle(p, a, b, c) || pointInTriangle(p, a, c, d);
}

// =============================================================== stroke font
// Each glyph is a list of line segments on a 4 (wide) x 6 (tall) grid.
// Segments are drawn as thin rotated quads with round caps.
static std::vector<glm::vec4> g_glyph[128];

static void G(char ch, std::initializer_list<float> pts)
{
    std::vector<float> p(pts);
    for (size_t i = 0; i + 3 < p.size(); i += 4)
        g_glyph[(int)ch].push_back(glm::vec4(p[i], p[i + 1], p[i + 2], p[i + 3]));
}

static void buildFont()
{
    G('A', { 0,0, 0,4,  0,4, 2,6,  2,6, 4,4,  4,4, 4,0,  0,2.8f, 4,2.8f });
    G('B', { 0,0, 0,6,  0,6, 3,6,  3,6, 4,5,  4,5, 4,4,  4,4, 3,3,  0,3, 3,3,  3,3, 4,2,  4,2, 4,1,  4,1, 3,0,  3,0, 0,0 });
    G('C', { 4,6, 1,6,  1,6, 0,5,  0,5, 0,1,  0,1, 1,0,  1,0, 4,0 });
    G('D', { 0,0, 0,6,  0,6, 2,6,  2,6, 4,4,  4,4, 4,2,  4,2, 2,0,  2,0, 0,0 });
    G('E', { 4,6, 0,6,  0,6, 0,0,  0,0, 4,0,  0,3, 3,3 });
    G('F', { 4,6, 0,6,  0,6, 0,0,  0,3, 3,3 });
    G('G', { 4,5, 3,6,  3,6, 1,6,  1,6, 0,5,  0,5, 0,1,  0,1, 1,0,  1,0, 3,0,  3,0, 4,1,  4,1, 4,3,  4,3, 2,3 });
    G('H', { 0,0, 0,6,  4,0, 4,6,  0,3, 4,3 });
    G('I', { 1,6, 3,6,  2,6, 2,0,  1,0, 3,0 });
    G('J', { 4,6, 4,1,  4,1, 3,0,  3,0, 1,0,  1,0, 0,1 });
    G('K', { 0,0, 0,6,  0,2, 4,6,  1.5f,3.5f, 4,0 });
    G('L', { 0,6, 0,0,  0,0, 4,0 });
    G('M', { 0,0, 0,6,  0,6, 2,3,  2,3, 4,6,  4,6, 4,0 });
    G('N', { 0,0, 0,6,  0,6, 4,0,  4,0, 4,6 });
    G('O', { 1,0, 0,1,  0,1, 0,5,  0,5, 1,6,  1,6, 3,6,  3,6, 4,5,  4,5, 4,1,  4,1, 3,0,  3,0, 1,0 });
    G('P', { 0,0, 0,6,  0,6, 3,6,  3,6, 4,5,  4,5, 4,4,  4,4, 3,3,  3,3, 0,3 });
    G('Q', { 1,0, 0,1,  0,1, 0,5,  0,5, 1,6,  1,6, 3,6,  3,6, 4,5,  4,5, 4,1,  4,1, 3,0,  3,0, 1,0,  2.5f,1.5f, 4.3f,-0.5f });
    G('R', { 0,0, 0,6,  0,6, 3,6,  3,6, 4,5,  4,5, 4,4,  4,4, 3,3,  3,3, 0,3,  2,3, 4,0 });
    G('S', { 4,5, 3,6,  3,6, 1,6,  1,6, 0,5,  0,5, 0,4,  0,4, 1,3,  1,3, 3,3,  3,3, 4,2,  4,2, 4,1,  4,1, 3,0,  3,0, 1,0,  1,0, 0,1 });
    G('T', { 0,6, 4,6,  2,6, 2,0 });
    G('U', { 0,6, 0,1,  0,1, 1,0,  1,0, 3,0,  3,0, 4,1,  4,1, 4,6 });
    G('V', { 0,6, 2,0,  2,0, 4,6 });
    G('W', { 0,6, 1,0,  1,0, 2,3,  2,3, 3,0,  3,0, 4,6 });
    G('X', { 0,6, 4,0,  0,0, 4,6 });
    G('Y', { 0,6, 2,3,  4,6, 2,3,  2,3, 2,0 });
    G('Z', { 0,6, 4,6,  4,6, 0,0,  0,0, 4,0 });
    G('0', { 1,0, 0,1,  0,1, 0,5,  0,5, 1,6,  1,6, 3,6,  3,6, 4,5,  4,5, 4,1,  4,1, 3,0,  3,0, 1,0,  0.8f,1.2f, 3.2f,4.8f });
    G('1', { 1,5, 2,6,  2,6, 2,0,  1,0, 3,0 });
    G('2', { 0,5, 1,6,  1,6, 3,6,  3,6, 4,5,  4,5, 4,4,  4,4, 0,0,  0,0, 4,0 });
    G('3', { 0,6, 4,6,  4,6, 2,3.5f,  2,3.5f, 3,3.5f,  3,3.5f, 4,2.5f,  4,2.5f, 4,1,  4,1, 3,0,  3,0, 1,0,  1,0, 0,1 });
    G('4', { 3,0, 3,6,  3,6, 0,2,  0,2, 4,2 });
    G('5', { 4,6, 0,6,  0,6, 0,3.5f,  0,3.5f, 3,3.5f,  3,3.5f, 4,2.5f,  4,2.5f, 4,1,  4,1, 3,0,  3,0, 0,0 });
    G('6', { 3.5f,6, 1,6,  1,6, 0,5,  0,5, 0,1,  0,1, 1,0,  1,0, 3,0,  3,0, 4,1,  4,1, 4,2.5f,  4,2.5f, 3,3.5f,  3,3.5f, 0,3.5f });
    G('7', { 0,6, 4,6,  4,6, 1.5f,0 });
    G('8', { 1,3, 0,4,  0,4, 0,5,  0,5, 1,6,  1,6, 3,6,  3,6, 4,5,  4,5, 4,4,  4,4, 3,3,  3,3, 1,3,  1,3, 0,2,  0,2, 0,1,  0,1, 1,0,  1,0, 3,0,  3,0, 4,1,  4,1, 4,2,  4,2, 3,3 });
    G('9', { 0.5f,0, 3,0,  3,0, 4,1,  4,1, 4,5,  4,5, 3,6,  3,6, 1,6,  1,6, 0,5,  0,5, 0,3.5f,  0,3.5f, 1,2.5f,  1,2.5f, 4,2.5f });
    G('!', { 2,6, 2,2,  2,0.2f, 2,0 });
    G('?', { 0,5, 1,6,  1,6, 3,6,  3,6, 4,5,  4,5, 4,4,  4,4, 2,2.6f,  2,2.6f, 2,1.8f,  2,0.2f, 2,0 });
    G('.', { 2,0.2f, 2,0 });
    G(',', { 2,0.4f, 1.4f,-1 });
    G('\'', { 2,6, 2,4.6f });
    G(':', { 2,4.2f, 2,4.0f,  2,0.2f, 2,0 });
    G('-', { 0.6f,3, 3.4f,3 });
    G('+', { 0.6f,3, 3.4f,3,  2,1.6f, 2,4.4f });
    G('/', { 0,0, 4,6 });
    G('<', { 4,6, 0,3,  0,3, 4,0 });
    G('>', { 0,6, 4,3,  4,3, 0,0 });
    G('=', { 0.5f,2, 3.5f,2,  0.5f,4, 3.5f,4 });
}

float textWidth(const std::string& s, float h)
{
    float unit = h / 6.0f;
    return s.size() > 0 ? (s.size() * 6.0f - 2.0f) * unit : 0.0f;
}

void drawText(const std::string& s, float x, float y, float h, Color c, float thick)
{
    float unit = h / 6.0f;
    if (thick < 0) thick = h * 0.13f;
    float penX = x;
    for (char ch : s) {
        char up = (ch >= 'a' && ch <= 'z') ? (char)(ch - 32) : ch;
        if (up > 0) {
            for (const glm::vec4& sg : g_glyph[(int)up]) {
                roundLine(penX + sg.x * unit, y + sg.y * unit,
                          penX + sg.z * unit, y + sg.w * unit, thick, c);
            }
        }
        penX += 6.0f * unit;
    }
}

void drawTextCentered(const std::string& s, float cx, float y, float h, Color c, float thick)
{
    drawText(s, cx - textWidth(s, h) * 0.5f, y, h, c, thick);
}
