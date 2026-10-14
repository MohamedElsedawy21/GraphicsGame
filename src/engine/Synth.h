#pragma once
#include <vector>

// Makes sounds from math instead of loading .wav files.
// The submission may only contain .h/.cpp files, so the game creates all its
// music and sound effects in code when it starts.
namespace Synth {
    const int SAMPLE_RATE = 44100;           // samples per second (CD quality)
    using Samples = std::vector<float>;      // one number per sample, -1 to +1

    enum class Wave { Sine, Square, Triangle, Noise };

    // Adds one note into 'out', starting at 'start' seconds.
    // The pitch slides from freqStart to freqEnd (use the same value for a steady note).
    void addNote(Samples& out, float start, float duration,
                 float freqStart, float freqEnd, Wave wave, float volume);

    // Frequency of a piano key. 69 = A4 = 440 Hz, +1 = one semitone higher.
    float noteFreq(int midiNote);

    // Keeps every sample between -1 and +1 (prevents crackling)
    void clip(Samples& s);
}
