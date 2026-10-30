// ============================================================================
//  audio.cpp : BONUS - background music + sound effects with volume control
// ============================================================================
#define _CRT_SECURE_NO_WARNINGS
#include "audio.h"
#include <string>
#include <cstdio>

static float g_volume = 0.7f;
static bool  g_muted = false;

#if USE_AUDIO
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

static ma_engine g_engine;
static ma_sound  g_music;
static bool      g_audioOk = false;
static bool      g_musicOk = false;
static std::string g_soundDir = "sounds/";

// find the sounds folder whether the game is started from Template/, the
// project root (.\build\game.exe) or the build folder itself
static void findSoundDir()
{
    const char* dirs[] = { "sounds/", "build/sounds/", "Template/sounds/", "../sounds/", "../Template/sounds/" };
    for (const char* d : dirs) {
        std::string test = std::string(d) + "music.wav";
        FILE* f = fopen(test.c_str(), "rb");
        if (f) { fclose(f); g_soundDir = d; return; }
    }
}

static void applyVolume()
{
    if (g_audioOk) ma_engine_set_volume(&g_engine, g_muted ? 0.0f : g_volume);
}

void audioInit()
{
    if (ma_engine_init(NULL, &g_engine) != MA_SUCCESS) return;
    g_audioOk = true;
    findSoundDir();
    std::string music = g_soundDir + "music.wav";
    if (ma_sound_init_from_file(&g_engine, music.c_str(), MA_SOUND_FLAG_STREAM, NULL, NULL, &g_music) == MA_SUCCESS) {
        g_musicOk = true;
        ma_sound_set_looping(&g_music, MA_TRUE);
        ma_sound_set_volume(&g_music, 0.55f);
        ma_sound_start(&g_music);
    }
    applyVolume();
}

void audioShutdown()
{
    if (g_musicOk) ma_sound_uninit(&g_music);
    if (g_audioOk) ma_engine_uninit(&g_engine);
}

void playSfx(const char* name)
{
    if (!g_audioOk || g_muted) return;
    std::string path = g_soundDir + name + ".wav";
    ma_engine_play_sound(&g_engine, path.c_str(), NULL);
}

#else
static void applyVolume() {}
void audioInit() {}
void audioShutdown() {}
void playSfx(const char*) {}
#endif

void volumeUp()   { g_volume += 0.1f; if (g_volume > 1.0f) g_volume = 1.0f; g_muted = false; applyVolume(); }
void volumeDown() { g_volume -= 0.1f; if (g_volume < 0.0f) g_volume = 0.0f; applyVolume(); }
void toggleMute() { g_muted = !g_muted; applyVolume(); }
float getVolume() { return g_volume; }
bool  isMuted()   { return g_muted; }
