// ============================================================================
//  models.h : every object of the scene, built from many primitives
//             (local coordinates: feet / wheels on y = 0, centred on x = 0)
//  Pedestrians face RIGHT (+x).  Vehicles face LEFT (-x).
// ============================================================================
#pragma once
#include "renderer.h"

// ------------------------------------------------------------- layout
const int   NUM_LANES   = 4;
const float LANE_BOTTOM = 78.0f;     // y of the bottom of lane 0
const float LANE_H      = 120.0f;
const float PF_LEFT     = 124.0f;    // home-base (metro station) edge
const float PF_RIGHT    = 1272.0f;   // enemy entry edge
const float CHAR_SCALE  = 0.85f;     // pedestrians/vehicles are drawn at this scale in game

inline float laneBottom(int lane) { return LANE_BOTTOM + lane * LANE_H; }
inline float laneGround(int lane) { return laneBottom(lane) + 16.0f; }

// ------------------------------------------------------------- animation pose
struct Pose {
    float t = 0.0f;        // personal timer (idle anims)
    float walk = 0.0f;     // walk/wheel phase
    float attack = 0.0f;   // 0..1 attack cycle
    bool  moving = false;
    float shake = 0.0f;    // hit shake
};

// players (pedestrians)
void drawVendor(const Pose& p);     // MELEE   : galabeya + "nabboot" stick
void drawKid(const Pose& p);        // RANGED  : kid with slingshot
void drawOfficer(const Pose& p);    // DEFENSE : traffic officer + metal barrier

// enemies (vehicles)
void drawTuktuk(const Pose& p);     // EASY    : black tuk-tuk throwing tires (high attack)
void drawMicrobus(const Pose& p);   // MEDIUM  : white microbus, kicks gravel along the ground (low attack)
void drawCar(const Pose& p);        // HARD    : black & white taxi (melee)

// small things
void drawShadow(float halfW);
void drawCoinModel(float t);
void drawPebble(float rot);
void drawTire(float rot);
void drawRock(float rot);
void drawHealthBar(float cx, float y, float w, int hp, int maxHp);
void drawPuff(float x, float y, float k);          // k: 0..1 life of the dust cloud
void drawShatter(float x, float y, float k);       // pebble breaking on a car

// scene
void drawSkyline(float t);
void drawStreet(float t);                           // sidewalks, asphalt
void drawLaneMarks();
void drawLaneHighlight(int lane, bool hovered, bool selected);
void drawEnvironment(float t);                      // kiosk, ful cart, bench, cat, bins
void drawMetroStation(float t);
void drawSpeechBubble(float x, float y, float w, float h, float tailX, float tailY, const std::string& l1, const std::string& l2);
