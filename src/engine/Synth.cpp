#include "engine/Synth.h"
#include <cmath>
#include <cstdint>

namespace {
    const float PI = 3.14159265f;

    // A tiny random number generator for noise (same result every run)
    uint32_t noiseState = 12345;
    float randomNoise() {
        noiseState = noiseState * 1664525u + 1013904223u;
        return ((noiseState >> 9) / 4194304.0f) - 1.0f;   // -1 to +1
    }

    // One sample of a wave shape. 'phase' goes 0 -> 1 once per cycle.
    float waveSample(Synth::Wave wave, float phase) {
        switch (wave) {
            case Synth::Wave::Sine:     return std::sin(2.0f * PI * phase);
            case Synth::Wave::Square:   return phase < 0.5f ? 1.0f : -1.0f;
            case Synth::Wave::Triangle: return 4.0f * std::fabs(phase - 0.5f) - 1.0f;
            case Synth::Wave::Noise:    return randomNoise();
        }
        return 0.0f;
    }
}

void Synth::addNote(Samples& out, float start, float duration,
                    float freqStart, float freqEnd, Wave wave, float volume) {
    int first = (int)(start * SAMPLE_RATE);
    int count = (int)(duration * SAMPLE_RATE);
    if ((int)out.size() < first + count) out.resize(first + count, 0.0f);

    const float attack = 0.005f;   // 5 ms fade-in so the note doesn't "click"
    float phase = 0.0f;

    for (int i = 0; i < count; i++) {
        float t = (float)i / SAMPLE_RATE;       // seconds since the note began
        float progress = (float)i / count;      // 0 -> 1 over the note

        // Volume shape (envelope): quick fade in, then fade out smoothly
        float envelope = (1.0f - progress) * (1.0f - progress);
        if (t < attack) envelope *= t / attack;

        // Pitch slides from start to end
        float freq = freqStart + (freqEnd - freqStart) * progress;
        phase += freq / SAMPLE_RATE;
        phase -= std::floor(phase);

        out[first + i] += waveSample(wave, phase) * envelope * volume;
    }
}

float Synth::noteFreq(int midiNote) {
    return 440.0f * std::pow(2.0f, (midiNote - 69) / 12.0f);
}

void Synth::clip(Samples& s) {
    for (float& v : s) {
        if (v > 1.0f)  v = 1.0f;
        if (v < -1.0f) v = -1.0f;
    }
}
