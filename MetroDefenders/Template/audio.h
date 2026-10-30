// ============================================================================
//  audio.h : BONUS sound (miniaudio single-header library)
//  Set USE_AUDIO to 0 to build the game without any sound.
// ============================================================================
#pragma once

#define USE_AUDIO 1

void  audioInit();
void  audioShutdown();
void  playSfx(const char* name);     // e.g. playSfx("coin")  -> sounds/coin.wav
void  volumeUp();
void  volumeDown();
void  toggleMute();
float getVolume();                   // 0..1
bool  isMuted();
