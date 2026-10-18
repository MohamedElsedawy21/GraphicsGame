#pragma once
#include "engine/Renderer.h"

// =====================================================================
//  Palette.h  -  EVERY color in the game lives here.
//  Rule for the team: never type color numbers inside a model or scene.
//  Use a name from this file, so the whole game keeps one consistent look.
//  Red is reserved for damage, so nothing below is pure red except DAMAGE hints.
// =====================================================================
namespace Palette {
    // ---- General ----
    inline constexpr Color OUTLINE      {0.10f, 0.09f, 0.10f};
    inline constexpr Color WHITE        {0.96f, 0.95f, 0.92f};
    inline constexpr Color BLACK        {0.08f, 0.08f, 0.09f};
    inline constexpr Color SHADOW       {0.00f, 0.00f, 0.00f, 0.25f};

    // ---- Sky and street (Persons 2 and 3) ----
    inline constexpr Color SKY_TOP      {0.98f, 0.80f, 0.55f};  // warm Cairo haze
    inline constexpr Color SKY_BOTTOM   {0.99f, 0.91f, 0.74f};
    inline constexpr Color SUN          {1.00f, 0.95f, 0.70f};
    inline constexpr Color ASPHALT      {0.30f, 0.30f, 0.33f};
    inline constexpr Color ASPHALT_DARK {0.24f, 0.24f, 0.27f};
    inline constexpr Color LANE_LINE    {0.93f, 0.90f, 0.80f};
    inline constexpr Color SIDEWALK     {0.72f, 0.66f, 0.58f};
    inline constexpr Color CURB         {0.55f, 0.50f, 0.45f};
    inline constexpr Color BUILDING_1   {0.85f, 0.72f, 0.55f};  // sand
    inline constexpr Color BUILDING_2   {0.76f, 0.60f, 0.47f};  // faded brick
    inline constexpr Color BUILDING_3   {0.66f, 0.62f, 0.58f};  // concrete
    inline constexpr Color WINDOW       {0.33f, 0.42f, 0.50f};
    inline constexpr Color SHUTTER      {0.40f, 0.55f, 0.45f};  // green wooden shutters
    inline constexpr Color LANE_HOVER   {1.00f, 1.00f, 1.00f, 0.12f};
    inline constexpr Color LANE_SELECT  {1.00f, 0.85f, 0.30f, 0.22f};

    // ---- Home base: metro station (Person 2) ----
    inline constexpr Color METRO_RED    {0.80f, 0.15f, 0.15f};  // the "M" sign (a sign, not damage)
    inline constexpr Color METRO_BLUE   {0.12f, 0.28f, 0.52f};
    inline constexpr Color METRO_TILE   {0.88f, 0.86f, 0.80f};

    // ---- Players (Person 4) ----
    inline constexpr Color SKIN         {0.78f, 0.58f, 0.42f};
    inline constexpr Color HAIR         {0.15f, 0.12f, 0.10f};
    inline constexpr Color SHIRT        {0.35f, 0.55f, 0.75f};  // pedestrian's blue shirt
    inline constexpr Color TROUSERS     {0.25f, 0.27f, 0.33f};
    inline constexpr Color SHOE         {0.30f, 0.20f, 0.14f};
    inline constexpr Color SLIPPER      {0.30f, 0.50f, 0.30f};  // the famous green "shib-shib"
    inline constexpr Color GALABEYA     {0.62f, 0.55f, 0.42f};  // stone thrower's robe
    inline constexpr Color HEAD_WRAP    {0.94f, 0.92f, 0.86f};
    inline constexpr Color POUCH        {0.50f, 0.38f, 0.25f};
    inline constexpr Color STONE        {0.55f, 0.53f, 0.50f};
    inline constexpr Color BARRIER_RED  {0.85f, 0.25f, 0.18f};  // stripes on the barrier
    inline constexpr Color BARRIER_WHITE{0.97f, 0.97f, 0.95f};
    inline constexpr Color METAL        {0.55f, 0.57f, 0.60f};
    inline constexpr Color AMBER        {1.00f, 0.70f, 0.10f};  // warning light, taillights

    // ---- Enemies (Person 4) ----
    inline constexpr Color CAR_BODY     {0.79f, 0.40f, 0.12f};  // rust orange #C9671F
    inline constexpr Color TUKTUK_BODY  {0.95f, 0.78f, 0.15f};  // yellow
    inline constexpr Color TUKTUK_CANOPY{0.20f, 0.20f, 0.24f};
    inline constexpr Color MICROBUS_BODY{0.92f, 0.92f, 0.88f};  // white
    inline constexpr Color MICROBUS_STRIPE{0.20f, 0.45f, 0.70f};
    inline constexpr Color TIRE         {0.12f, 0.12f, 0.13f};
    inline constexpr Color HUBCAP       {0.70f, 0.72f, 0.75f};
    inline constexpr Color GLASS        {0.35f, 0.50f, 0.60f, 0.85f};
    inline constexpr Color HEADLIGHT    {1.00f, 0.95f, 0.60f};
    inline constexpr Color WATERMELON   {0.25f, 0.55f, 0.25f};
    inline constexpr Color CRATE        {0.66f, 0.48f, 0.28f};
    inline constexpr Color SEAT         {0.55f, 0.22f, 0.25f};
    inline constexpr Color BEARD        {0.32f, 0.29f, 0.27f};
    inline constexpr Color BUS_BODY     {0.18f, 0.48f, 0.40f};  // cutscene bus: green
    inline constexpr Color BUS_TOP      {0.93f, 0.91f, 0.84f};  // cream upper half
    inline constexpr Color TREAD        {0.26f, 0.26f, 0.28f};

    // ---- Collectible: Egyptian pound coin (Person 1) ----
    inline constexpr Color COIN_GOLD    {0.90f, 0.72f, 0.25f};  // brass outer ring
    inline constexpr Color COIN_GOLD_DARK{0.70f, 0.52f, 0.15f};
    inline constexpr Color COIN_SILVER  {0.82f, 0.83f, 0.85f};  // nickel center
    inline constexpr Color COIN_SILVER_DARK{0.58f, 0.60f, 0.63f};
    inline constexpr Color COIN_BLUE    {0.20f, 0.35f, 0.60f};  // stripes on the emblem

    // ---- UI: HUD, cards, menus (Person 1) ----
    inline constexpr Color UI_PANEL     {0.20f, 0.16f, 0.13f};  // dark wood
    inline constexpr Color UI_PANEL_LIGHT{0.32f, 0.26f, 0.20f};
    inline constexpr Color UI_CARD      {0.93f, 0.86f, 0.70f};  // papyrus
    inline constexpr Color UI_CARD_HOVER{1.00f, 0.95f, 0.80f};
    inline constexpr Color UI_SELECT    {1.00f, 0.80f, 0.20f};
    inline constexpr Color UI_TEXT      {0.98f, 0.95f, 0.88f};
    inline constexpr Color UI_TEXT_DARK {0.20f, 0.16f, 0.13f};
    inline constexpr Color UI_BUTTON    {0.15f, 0.42f, 0.55f};  // Nile blue
    inline constexpr Color UI_BUTTON_HOVER{0.22f, 0.55f, 0.70f};
    inline constexpr Color UI_DIM       {0.00f, 0.00f, 0.00f, 0.55f};  // darkens the game behind menus
    inline constexpr Color HEALTH_GOOD  {0.30f, 0.80f, 0.35f};
    inline constexpr Color HEALTH_BACK  {0.15f, 0.15f, 0.15f};
}
