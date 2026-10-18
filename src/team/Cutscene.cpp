// =====================================================================
//  OWNER: Person 4  -  the intro cutscene (about 20 seconds).
//
//  Story: Am Sayed walks to the metro station on a calm Cairo morning.
//  A tuk-tuk cuts him off, a microbus honks him away, a car nearly runs
//  him over, and a huge bus blocks the street, claiming the street and the
//  metro station. Am Sayed raises his slipper: "This is war!"
//  -> the player must defend the metro station from the vehicles.
//
//  How it works:
//    - every actor follows a TRACK of keyframes (time, position, rotation, scale)
//    - the camera is a track too (it zooms in on the hero at the end)
//    - LINES are the dialogue: who speaks, when, the text, and the sound.
//      People "talk" in gibberish babble (cartoon style) while their words
//      appear in a speech bubble; vehicles honk and get a comic burst bubble.
// =====================================================================
#include "team/Cutscene.h"
#include "team/Models.h"
#include "engine/Renderer.h"
#include "engine/Text.h"
#include "engine/Audio.h"
#include "shared/Palette.h"
#include "ui/CoinModel.h"
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>

namespace {
    const float LENGTH = 19.8f;    // seconds (the spec asks for 10-20)
    const float PI = 3.14159265f;
    float timer = 0.0f;

    // =================================================================
    //  Section A: the keyframe timeline
    // =================================================================
    struct Pose { float x, y, rot, scale; };

    // How to move from one key to the next
    enum class Ease { Linear, Smooth, Out, In };

    struct Key {
        float t;      // time in seconds
        Pose  pose;   // where the actor is at that time
        Ease  ease;   // how to travel to the NEXT key
    };

    float applyEase(Ease e, float k) {
        switch (e) {
            case Ease::Linear: return k;
            case Ease::Smooth: return k * k * (3.0f - 2.0f * k);          // slow - fast - slow
            case Ease::Out:    return 1.0f - (1.0f - k) * (1.0f - k);     // fast, then brakes
            case Ease::In:     return k * k;                              // starts slow, speeds up
        }
        return k;
    }

    // Where is the actor at time t? Find the two keys around t and blend between them.
    Pose sample(const std::vector<Key>& keys, float t) {
        if (t <= keys.front().t) return keys.front().pose;
        if (t >= keys.back().t)  return keys.back().pose;
        for (size_t i = 0; i + 1 < keys.size(); i++) {
            const Key& a = keys[i];
            const Key& b = keys[i + 1];
            if (t >= a.t && t <= b.t) {
                float k = applyEase(a.ease, (t - a.t) / (b.t - a.t));
                return { a.pose.x + (b.pose.x - a.pose.x) * k,
                         a.pose.y + (b.pose.y - a.pose.y) * k,
                         a.pose.rot + (b.pose.rot - a.pose.rot) * k,
                         a.pose.scale + (b.pose.scale - a.pose.scale) * k };
            }
        }
        return keys.back().pose;
    }

    bool isMoving(const std::vector<Key>& keys, float t) {
        return std::fabs(sample(keys, t + 0.05f).x - sample(keys, t).x) > 0.3f;
    }

    // ---- The tracks ----
    const float HERO_Y = 118.0f;   // the hero walks on the front sidewalk
    const float ROAD_Y = 182.0f;   // vehicles drive on the road behind him

    const std::vector<Key> HERO = {
        {  0.0f, { 760, HERO_Y,      0,    1.3f }, Ease::Linear },  // walking to the metro
        {  3.0f, { 520, HERO_Y,      0,    1.3f }, Ease::Smooth },
        {  3.85f,{ 520, HERO_Y,      0,    1.3f }, Ease::Out    },
        {  4.05f,{ 555, HERO_Y + 40, 10,   1.3f }, Ease::In     },  // jumps back in fright
        {  4.3f, { 570, HERO_Y,      0,    1.3f }, Ease::Smooth },
        { 12.65f,{ 570, HERO_Y,      0,    1.3f }, Ease::Smooth },
        { 13.15f,{ 570, HERO_Y,   -360,    1.3f }, Ease::Smooth },  // spun around by the speeding car
        { 16.2f, { 570, HERO_Y,   -360,    1.3f }, Ease::Smooth },
        { 16.8f, { 600, HERO_Y,   -360,    1.3f }, Ease::Smooth },  // steps forward to fight
    };

    const std::vector<Key> TUKTUK = {
        {  2.6f, { 1450, ROAD_Y,      0, 1.2f }, Ease::Linear },
        {  3.45f,{  760, ROAD_Y,      0, 1.2f }, Ease::Out    },
        {  3.9f, {  410, ROAD_Y - 34, -7, 1.2f }, Ease::Smooth },   // swerves in front of him
        {  4.3f, {  390, ROAD_Y - 20, 0, 1.2f }, Ease::Smooth },
        {  6.7f, {  390, ROAD_Y - 20, 0, 1.2f }, Ease::In     },
        {  7.6f, { -260, ROAD_Y,      0, 1.2f }, Ease::Linear },   // zooms off toward the metro
    };

    const std::vector<Key> MICROBUS = {
        {  7.6f, { 1560, ROAD_Y + 4, 0, 1.15f }, Ease::Out    },
        {  8.7f, { 1150, ROAD_Y + 4, 0, 1.15f }, Ease::Smooth },
    };

    const std::vector<Key> CAR = {
        { 12.3f, { 1500, ROAD_Y - 6, 0, 1.15f }, Ease::Linear },
        { 13.3f, { -350, ROAD_Y - 6, 0, 1.15f }, Ease::Linear },
    };

    const std::vector<Key> BUS = {
        { 13.4f, { 1750, ROAD_Y + 2, 0, 1.15f }, Ease::Out    },
        { 14.6f, {  860, ROAD_Y + 2, 0, 1.15f }, Ease::Smooth },
    };

    // Camera: x, y = the point at the center of the screen; scale = zoom
    const std::vector<Key> CAMERA = {
        {  0.0f, { 640, 360, 0, 1.0f  }, Ease::Smooth },
        { 15.9f, { 640, 360, 0, 1.0f  }, Ease::Smooth },
        { 16.5f, { 590, 250, 0, 1.55f }, Ease::Smooth },   // zoom in on the hero
        { 17.7f, { 590, 250, 0, 1.55f }, Ease::Smooth },
        { 18.2f, { 640, 360, 0, 1.0f  }, Ease::Smooth },
    };

    // =================================================================
    //  Section B: the dialogue
    // =================================================================
    enum class Who { Hero, TuktukDriver, MicrobusDriver, BusDriver,
                     Tuktuk, Microbus, Car, Bus, Narrator };

    struct Line {
        float start, end;
        Who who;
        const char* text;
        Sfx sound;      // Sfx::Count = no sound
        bool noise;     // true = a vehicle noise (comic burst bubble)
    };

    const Line LINES[] = {
        {  0.4f,  2.9f, Who::Hero,           "AHH... A CALM DAY IN CAIRO. TIME TO TAKE THE METRO HOME.", Sfx::TalkHero, false },
        {  3.0f,  4.0f, Who::Tuktuk,         "BEEP BEEP!",                                Sfx::HornTuktuk,      true  },
        {  4.4f,  6.6f, Who::TuktukDriver,   "MOVE IT, YA BASHA! THIS STREET IS MINE!",   Sfx::TalkDriver,      false },
        {  6.7f,  7.9f, Who::Hero,           "HEY! I WAS WALKING HERE!",                  Sfx::TalkHeroExcited, false },
        {  8.0f,  9.1f, Who::Microbus,       "HOOONK! HONK-HONK!",                        Sfx::HornMicrobus,    true  },
        {  9.2f, 11.4f, Who::MicrobusDriver, "RAMSIS! RAMSIS! GET IN OR GET OUT OF THE WAY!", Sfx::TalkDriver,  false },
        { 11.5f, 12.3f, Who::Hero,           "SERIOUSLY?!",                               Sfx::TalkHeroExcited, false },
        { 12.3f, 13.3f, Who::Car,            "VROOOOOM!",                                 Sfx::CarZoom,         true  },
        { 13.6f, 14.6f, Who::Bus,            "HOOOOOOONK!!",                              Sfx::HornBus,         true  },
        { 14.7f, 16.1f, Who::BusDriver,      "THIS STREET AND THE METRO ARE OURS NOW!",   Sfx::TalkBig,         false },
        { 16.2f, 17.8f, Who::Hero,           "THAT'S IT... THIS IS WAR!",                 Sfx::TalkHeroShout,   false },
        { 17.9f, 19.8f, Who::Narrator,       "DEFEND THE METRO STATION!",                 Sfx::Count,           false },
    };

    // Extra sound effects that are not dialogue
    struct Cue { float t; Sfx sound; };
    const Cue CUES[] = {
        {  3.75f, Sfx::Screech },   // tuk-tuk swerves
        { 14.2f,  Sfx::Screech },   // bus brakes
        { 17.9f,  Sfx::Win     },   // title fanfare
    };

    const char* speakerName(Who w) {
        switch (w) {
            case Who::Hero:           return "AM SAYED";
            case Who::TuktukDriver:   return "TUK-TUK DRIVER";
            case Who::MicrobusDriver: return "MICROBUS DRIVER";
            case Who::BusDriver:      return "BUS DRIVER";
            case Who::Tuktuk:         return "TUK-TUK";
            case Who::Microbus:       return "MICROBUS";
            case Who::Car:            return "CAR";
            case Who::Bus:            return "BUS";
            default:                  return "";
        }
    }

    const float TYPE_SPEED = 38.0f;   // letters per second (typewriter effect)

    bool isTalking(Who who, float t) {
        for (const Line& l : LINES) {
            if (l.who != who || l.noise) continue;
            float typingEnd = l.start + std::string(l.text).size() / TYPE_SPEED + 0.2f;
            if (t >= l.start && t < typingEnd && t < l.end) return true;
        }
        return false;
    }

    bool lineActive(Who who, float t) {
        for (const Line& l : LINES) if (l.who == who && t >= l.start && t < l.end) return true;
        return false;
    }

    // Split a sentence into lines of at most maxChars letters (word wrap)
    std::vector<std::string> wrap(const std::string& text, size_t maxChars) {
        std::vector<std::string> lines;
        std::string line, word;
        for (size_t i = 0; i <= text.size(); i++) {
            char c = (i < text.size()) ? text[i] : ' ';
            if (c != ' ') { word += c; continue; }
            if (!line.empty() && line.size() + 1 + word.size() > maxChars) {
                lines.push_back(line);
                line.clear();
            }
            if (!word.empty()) line += (line.empty() ? "" : " ") + word;
            word.clear();
        }
        if (!line.empty()) lines.push_back(line);
        return lines;
    }

    // =================================================================
    //  Section C: the street (background)
    // =================================================================
    Color mixColor(const Color& a, const Color& b, float t) {
        return { a.r + (b.r - a.r) * t, a.g + (b.g - a.g) * t, a.b + (b.b - a.b) * t, 1.0f };
    }

    void drawSky(float t) {
        Color top    {0.55f, 0.78f, 0.93f};
        Color bottom {0.99f, 0.88f, 0.70f};   // warm morning haze near the horizon
        for (int i = 0; i < 24; i++) {
            float y = 250.0f + i * 20.0f;
            Renderer::drawRect(0, y, 1280, 21, mixColor(bottom, top, i / 23.0f));
        }
        Renderer::drawCircle(1080, 600, 70, {1.0f, 0.95f, 0.7f, 0.3f});
        Renderer::drawCircle(1080, 600, 42, Palette::SUN);

        // Drifting clouds
        for (int i = 0; i < 3; i++) {
            float cx = std::fmod(200.0f + i * 430.0f + t * 12.0f, 1500.0f) - 100.0f;
            float cy = 610.0f - i * 35.0f;
            Color cloud {1, 1, 1, 0.8f};
            Renderer::drawEllipse(cx, cy, 50, 16, cloud);
            Renderer::drawEllipse(cx - 28, cy + 8, 26, 14, cloud);
            Renderer::drawEllipse(cx + 22, cy + 10, 30, 16, cloud);
        }
        // A few birds
        for (int i = 0; i < 3; i++) {
            float bx = 300 + i * 40 + t * 25.0f, by = 560 + (i % 2) * 15.0f;
            float flap = std::sin(t * 10 + i) * 4;
            Renderer::drawLine(bx - 7, by + flap, bx, by, 2, Palette::OUTLINE);
            Renderer::drawLine(bx, by, bx + 7, by + flap, 2, Palette::OUTLINE);
        }
    }

    void drawBuilding(float x, float w, float h, const Color& wall, int seed) {
        float base = 255.0f;
        Renderer::setOutline(2.0f, Palette::OUTLINE);
        Renderer::drawRect(x, base, w, h, wall);
        Renderer::drawRect(x - 4, base + h, w + 8, 7, mixColor(wall, Palette::OUTLINE, 0.25f));
        Renderer::setOutline(0.0f, Palette::OUTLINE);

        // Floors of windows with green shutters and little balconies
        int floor = 0;
        for (float wy = base + 24; wy < base + h - 34; wy += 48, floor++) {
            for (float wx = x + 14; wx < x + w - 30; wx += 40) {
                Renderer::drawRect(wx, wy, 22, 28, Palette::WINDOW);
                Renderer::drawRect(wx - 5, wy, 5, 28, Palette::SHUTTER);
                Renderer::drawRect(wx + 22, wy, 5, 28, Palette::SHUTTER);
                if ((int)(wx + floor + seed) % 3 == 0) {   // some windows have a balcony
                    Renderer::drawRect(wx - 7, wy - 4, 36, 4, mixColor(wall, Palette::OUTLINE, 0.4f));
                    for (float r = wx - 5; r < wx + 29; r += 6)
                        Renderer::drawLine(r, wy, r, wy + 10, 1.5f, Palette::OUTLINE);
                    Renderer::drawLine(wx - 7, wy + 10, wx + 29, wy + 10, 1.5f, Palette::OUTLINE);
                } else if ((int)(wx + seed) % 4 == 1) {    // or an air conditioner
                    Renderer::drawRect(wx + 2, wy - 9, 18, 9, Palette::METAL);
                    Renderer::drawLine(wx + 4, wy - 5, wx + 18, wy - 5, 1, Palette::OUTLINE);
                }
            }
        }
        // Satellite dish on the roof
        float dx = x + w * 0.7f, dy = base + h + 7;
        Renderer::drawLine(dx, dy, dx, dy + 10, 2, Palette::OUTLINE);
        Renderer::pushTransform();
        Renderer::translate(dx, dy + 14);
        Renderer::rotate(25);
        Renderer::drawEllipse(0, 0, 9, 4, Palette::WHITE);
        Renderer::popTransform();
    }

    void drawCity() {
        // A minaret far behind the buildings
        Color far {0.80f, 0.70f, 0.62f};
        Renderer::drawRect(668, 255, 22, 320, far);
        Renderer::drawRect(662, 500, 34, 8, far);
        Renderer::drawTriangle(668, 575, 690, 575, 679, 615, far);

        drawBuilding(250, 190, 290, Palette::BUILDING_1, 1);
        drawBuilding(450, 170, 230, Palette::BUILDING_2, 2);
        drawBuilding(720, 210, 270, Palette::BUILDING_3, 3);
        drawBuilding(940, 160, 210, Palette::BUILDING_1, 4);
        drawBuilding(1110, 190, 300, Palette::BUILDING_2, 5);

        // Streetlights on the far sidewalk
        for (float lx : { 420.0f, 900.0f, 1250.0f }) {
            Renderer::drawLine(lx, 250, lx, 380, 4, Palette::OUTLINE);
            Renderer::drawLine(lx, 380, lx + 22, 380, 3, Palette::OUTLINE);
            Renderer::drawEllipse(lx + 24, 376, 8, 4, Palette::HEADLIGHT);
        }
    }

    void drawRoadAndSidewalk() {
        // Far sidewalk + road
        Renderer::drawRect(0, 250, 1280, 10, Palette::SIDEWALK);
        Renderer::drawRect(0, 160, 1280, 92, Palette::ASPHALT);
        for (float x = -20; x < 1280; x += 80)
            Renderer::drawRect(x, 204, 44, 5, Palette::LANE_LINE);

        // Front sidewalk with tiles
        Renderer::drawRect(0, 0, 1280, 160, Palette::SIDEWALK);
        for (float x = 0; x < 1280; x += 64) Renderer::drawLine(x, 0, x, 148, 1.5f, Palette::CURB);
        for (float y = 40; y < 148; y += 40) Renderer::drawLine(0, y, 1280, y, 1.5f, Palette::CURB);

        // Cairo curb painted black and white
        for (int i = 0; i < 32; i++) {
            Color c = (i % 2 == 0) ? Palette::WHITE : Palette::BLACK;
            Renderer::drawRect(i * 40.0f, 148, 40, 12, c);
        }
    }

    void drawMetroStation() {
        // Building and glass walls
        Renderer::setOutline(2.0f, Palette::OUTLINE);
        Renderer::drawRect(10, 118, 220, 14, Palette::CURB);                 // platform
        Renderer::drawRect(20, 132, 200, 120, Palette::METRO_TILE);           // walls
        Renderer::drawRect(70, 132, 100, 92, {0.20f, 0.22f, 0.26f});          // entrance
        Renderer::drawTrapezoid(0, 252, 240, 210, 30, Palette::METRO_BLUE);  // roof
        Renderer::setOutline(0.0f, Palette::OUTLINE);

        // Stairs going down into the entrance
        for (int i = 0; i < 6; i++) {
            float sy = 140 + i * 12.0f;
            float inset = i * 5.0f;
            Renderer::drawRect(78 + inset, sy, 84 - inset * 2, 4, {0.45f, 0.47f, 0.52f});
        }
        // Glass panels
        for (float gx : { 28.0f, 182.0f }) {
            Renderer::drawRect(gx, 150, 30, 80, Palette::GLASS);
            Renderer::drawLine(gx + 6, 160, gx + 14, 200, 2, {1, 1, 1, 0.35f});
        }
        Text::drawCentered("METRO", 120, 260, 14, Palette::WHITE);

        // The red "M" sign on a pole
        Renderer::drawLine(232, 118, 232, 330, 5, Palette::METAL);
        Renderer::setOutline(2.5f, Palette::OUTLINE);
        Renderer::drawCircle(232, 350, 26, Palette::METRO_RED);
        Renderer::setOutline(0.0f, Palette::OUTLINE);
        Text::drawCentered("M", 232, 337, 26, Palette::WHITE);
    }

    // =================================================================
    //  Section D: speech bubbles and noise bursts
    // =================================================================
    // A comic-book "burst" for vehicle noises: spiky star + big wobbly text
    void drawBurst(float cx, float cy, const std::string& text, float age) {
        float size = 18.0f;
        float w = Text::width(text, size) + 40.0f;
        float h = size + 30.0f;
        float pop = std::fmin(1.0f, age * 8.0f);              // pops in quickly
        float wobble = 1.0f + 0.05f * std::sin(age * 35.0f);

        Renderer::pushTransform();
        Renderer::translate(cx, cy);
        Renderer::scale(pop * wobble, pop * wobble);
        Renderer::rotate(std::sin(age * 20.0f) * 3.0f);

        // Draw twice: a bigger dark star (the outline), then the yellow star on top
        for (int pass = 0; pass < 2; pass++) {
            float grow = (pass == 0) ? 4.0f : 0.0f;
            Color c = (pass == 0) ? Palette::OUTLINE : Color{1.0f, 0.93f, 0.45f};
            Renderer::drawEllipse(0, 0, w / 2 + grow, h / 2 + grow, c);
            const int spikes = 14;
            for (int i = 0; i < spikes; i++) {
                float a = 2.0f * PI * i / spikes;
                float tip = (i % 2 == 0) ? 1.45f : 1.25f;
                float rx = w / 2, ry = h / 2;
                Renderer::drawTriangle(std::cos(a - 0.16f) * (rx * 0.9f + grow), std::sin(a - 0.16f) * (ry * 0.9f + grow),
                                       std::cos(a + 0.16f) * (rx * 0.9f + grow), std::sin(a + 0.16f) * (ry * 0.9f + grow),
                                       std::cos(a) * (rx * tip + grow * 1.6f), std::sin(a) * (ry * tip + grow * 1.6f), c);
            }
        }
        Text::drawCentered(text, 2, -size / 2 - 2, size, {0.75f, 0.25f, 0.05f});
        Text::drawCentered(text, 0, -size / 2, size, Palette::OUTLINE);
        Renderer::popTransform();
    }

    // A white speech bubble whose tail points at the speaker. The text types itself out.
    void drawSpeech(float ax, float ay, Who who, const std::string& full, float age) {
        const float size = 13.0f;
        std::vector<std::string> rows = wrap(full, 26);
        size_t maxLen = 0;
        for (const std::string& r : rows) maxLen = std::max(maxLen, r.size());

        float w = Text::width(std::string(maxLen, 'M'), size) + 30.0f;
        float rowH = size + 9.0f;
        float h = rows.size() * rowH + 18.0f;

        // Keep the bubble on screen; it sits above the speaker
        float bx = std::fmax(20.0f, std::fmin(1260.0f - w, ax - w / 2));
        float by = ay + 26.0f;
        float pop = std::fmin(1.0f, age * 7.0f);

        Renderer::pushTransform();
        Renderer::translate(ax, ay);
        Renderer::scale(pop, pop);
        Renderer::translate(-ax, -ay);

        // Tail + bubble (outline drawn by a slightly bigger dark shape behind)
        Renderer::drawTriangle(ax - 12, by + 3, ax + 12, by + 3, ax, ay - 2, Palette::OUTLINE);
        Renderer::drawRoundedRect(bx - 3, by - 3, w + 6, h + 6, 16, Palette::OUTLINE);
        Renderer::drawRoundedRect(bx, by, w, h, 14, Palette::WHITE);
        Renderer::drawTriangle(ax - 8, by + 4, ax + 8, by + 4, ax, ay + 3, Palette::WHITE);

        // Name tag on the top-left corner
        std::string name = speakerName(who);
        float tagW = Text::width(name, 9) + 16;
        Color tagColor = (who == Who::Hero) ? Palette::COIN_GOLD_DARK : Palette::CAR_BODY;
        Renderer::setOutline(2.0f, Palette::OUTLINE);
        Renderer::drawRoundedRect(bx + 10, by + h - 8, tagW, 18, 6, tagColor);
        Renderer::setOutline(0.0f, Palette::OUTLINE);
        Text::draw(name, bx + 18, by + h - 3, 9, Palette::WHITE);

        // Typewriter: show only as many letters as time allows
        int shown = (int)(age * TYPE_SPEED);
        float rowY = by + h - 22.0f - size;
        for (const std::string& r : rows) {
            int n = std::max(0, std::min((int)r.size(), shown));
            Text::draw(r.substr(0, n), bx + 15, rowY, size, Palette::UI_TEXT_DARK);
            shown -= (int)r.size() + 1;
            rowY -= rowH;
        }
        Renderer::popTransform();
    }

    // =================================================================
    //  Section E: drawing one actor along its track
    // =================================================================
    using ModelFn = void (*)(float, float, float, const AnimState&);

    void drawActor(ModelFn model, const Pose& p, bool faceLeft, float pivotHeight,
                   const AnimState& anim) {
        Renderer::pushTransform();
        // Rotate around the middle of the body, not the feet
        Renderer::translate(p.x, p.y + pivotHeight * p.scale);
        Renderer::rotate(p.rot);
        Renderer::translate(0, -pivotHeight * p.scale);
        Renderer::scale(faceLeft ? -p.scale : p.scale, p.scale);   // negative x = mirror
        model(0, 0, 1.0f, anim);
        Renderer::popTransform();
    }

    AnimState animFor(const std::vector<Key>& track, Who talker, float t) {
        AnimState a;
        a.time = t;
        a.moving = isMoving(track, t);
        a.talking = isTalking(talker, t);
        return a;
    }

    // Small up-down shake while a vehicle is honking
    Pose shaken(Pose p, Who noise, float t) {
        if (lineActive(noise, t)) p.y += std::fabs(std::sin(t * 40.0f)) * 2.5f;
        return p;
    }

    // Where each speaker's head is (the bubble tail points there)
    Vec2 anchorFor(Who who, float t) {
        Pose p;
        switch (who) {
            case Who::Hero: {
                p = sample(HERO, t);
                return { p.x, p.y + 118 * p.scale };
            }
            case Who::TuktukDriver: case Who::Tuktuk:
                p = sample(TUKTUK, t);   return { p.x - 28 * p.scale, p.y + 108 * p.scale };
            case Who::MicrobusDriver: case Who::Microbus:
                p = sample(MICROBUS, t); return { p.x - 60 * p.scale, p.y + 114 * p.scale };
            case Who::Car:
                p = sample(CAR, t);      return { p.x, p.y + 90 * p.scale };
            case Who::BusDriver: case Who::Bus:
                p = sample(BUS, t);      return { p.x - 120 * p.scale, p.y + 155 * p.scale };
            default: return { 640, 400 };
        }
    }

    // Dust clouds kicked up by the speeding car
    void drawDust(float t) {
        for (int i = 0; i < 10; i++) {
            float born = 12.3f + i * 0.09f;
            float age = t - born;
            if (age < 0.0f || age > 0.9f) continue;
            Pose p = sample(CAR, born);
            float r = 8 + age * 30;
            Renderer::drawCircle(p.x + 70 + age * 30, p.y + 10 + age * 25, r,
                                 {0.85f, 0.78f, 0.65f, 0.55f * (1.0f - age / 0.9f)});
        }
    }

    // =================================================================
    //  Section F: title card, letterbox, fades
    // =================================================================
    void drawTitleCard(float t) {
        float age = t - 17.9f;
        if (age < 0) return;
        float a = std::fmin(1.0f, age * 2.5f);
        Renderer::drawRect(0, 250, 1280, 230, {0.0f, 0.0f, 0.0f, 0.6f * a});

        float size = 40.0f;
        std::string title = "DEFEND THE METRO STATION!";
        float slide = (1.0f - a) * 60.0f;
        Text::drawShadowed(title, 640 - Text::width(title, size) / 2.0f - slide, 380, size,
                           {1.0f, 0.82f, 0.25f, a}, {0.0f, 0.0f, 0.0f, a});
        Text::drawCentered("THE VEHICLES WANT TO TAKE OVER THE METRO STATION.", 640, 330, 15,
                           {1, 1, 1, a});
        Text::drawCentered("PLACE DEFENDERS AND DON'T LET ANY VEHICLE REACH IT!", 640, 302, 15,
                           {1, 1, 1, a});
        drawCoin(640 - Text::width(title, size) / 2.0f - 45, 400, 22 * a, t);
        drawCoin(640 + Text::width(title, size) / 2.0f + 45, 400, 22 * a, t + 0.4f);
    }
}

// =====================================================================
//  Public functions (called by the Game)
// =====================================================================
void Cutscene::start() {
    timer = 0.0f;
}

bool Cutscene::update(float dt) {
    float before = timer;
    timer += dt;

    // Play every sound whose start time we just passed
    for (const Line& l : LINES) {
        if (l.sound != Sfx::Count && before < l.start && timer >= l.start) Audio::play(l.sound);
    }
    for (const Cue& c : CUES) {
        if (before < c.t && timer >= c.t) Audio::play(c.sound);
    }
    return timer >= LENGTH;
}

void Cutscene::draw() {
    float t = timer;

    // ---- Camera: zoom + a little shake when the big vehicles honk ----
    Pose cam = sample(CAMERA, t);
    float shake = lineActive(Who::Bus, t) ? 4.0f : (lineActive(Who::Microbus, t) ? 1.5f : 0.0f);
    Renderer::pushTransform();
    Renderer::translate(640 + std::sin(t * 70.0f) * shake, 360 + std::cos(t * 53.0f) * shake);
    Renderer::scale(cam.scale, cam.scale);
    Renderer::translate(-cam.x, -cam.y);

    // ---- The street, back to front ----
    drawSky(t);
    drawCity();
    drawRoadAndSidewalk();
    drawMetroStation();

    // ---- Vehicles (on the road, behind the hero). They all face LEFT. ----
    drawActor(Models::drawMicrobus, shaken(sample(MICROBUS, t), Who::Microbus, t), true, 55,
              animFor(MICROBUS, Who::MicrobusDriver, t));
    drawActor(Models::drawTuktuk, shaken(sample(TUKTUK, t), Who::Tuktuk, t), true, 50,
              animFor(TUKTUK, Who::TuktukDriver, t));
    drawDust(t);
    drawActor(Models::drawCar, sample(CAR, t), true, 40, animFor(CAR, Who::Car, t));
    drawActor(Models::drawBus, shaken(sample(BUS, t), Who::Bus, t), true, 75,
              animFor(BUS, Who::BusDriver, t));

    // ---- The hero: faces the metro (left) at first, then turns to face the traffic ----
    AnimState hero = animFor(HERO, Who::Hero, t);
    if (t > 16.2f) {   // raises the slipper over his head and shakes it
        hero.attack = std::fmin(1.0f, (t - 16.2f) / 0.6f) * 0.42f + std::sin(t * 12.0f) * 0.02f;
    }
    drawActor(Models::drawPedestrian, sample(HERO, t), t < 4.0f, 50, hero);

    // ---- Speech bubbles and honk bursts (the "subtitles") ----
    for (const Line& l : LINES) {
        if (t < l.start || t >= l.end || l.who == Who::Narrator) continue;
        Vec2 a = anchorFor(l.who, t);
        if (l.noise) drawBurst(std::fmax(110.0f, std::fmin(1170.0f, a.x)), a.y + 40, l.text, t - l.start);
        else         drawSpeech(a.x, a.y, l.who, l.text, t - l.start);
    }
    Renderer::popTransform();

    // ---- Screen-space overlays: title card, movie bars, fades ----
    drawTitleCard(t);
    Renderer::drawRect(0, 0, 1280, 44, Palette::BLACK);
    Renderer::drawRect(0, 676, 1280, 44, Palette::BLACK);

    float fade = 0.0f;
    if (t < 0.6f) fade = 1.0f - t / 0.6f;
    if (t > LENGTH - 0.5f) fade = (t - (LENGTH - 0.5f)) / 0.5f;
    if (fade > 0.0f) Renderer::drawRect(0, 0, 1280, 720, {0, 0, 0, fade});
}
