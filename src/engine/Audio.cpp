#include "engine/Audio.h"
#include "engine/Synth.h"
#include "miniaudio.h"
#include <iostream>
#include <cmath>

using Synth::Wave;

namespace {
    // ===== Section A: playback state =====
    const int VOICES = 4;   // the same effect can overlap up to 4 times

    struct SoundSlot {
        Synth::Samples   samples;              // the sound itself
        ma_audio_buffer  buffers[VOICES];      // miniaudio's view of the samples
        ma_sound         voices[VOICES];       // things miniaudio can play
        int              nextVoice = 0;
    };

    ma_engine engine;
    bool ready = false;

    SoundSlot effects[(int)Sfx::Count];
    SoundSlot music;

    float masterVolume = 0.7f;
    bool  muted = false;

    void applyVolume() {
        masterVolume = std::round(masterVolume * 10.0f) / 10.0f;   // keep exact steps of 10%
        if (ready) ma_engine_set_volume(&engine, muted ? 0.0f : masterVolume);
    }

    // ===== Section B: turn samples into playable sounds =====
    bool prepare(SoundSlot& slot, int voices) {
        Synth::clip(slot.samples);
        for (int v = 0; v < voices; v++) {
            ma_audio_buffer_config cfg = ma_audio_buffer_config_init(
                ma_format_f32, 1, slot.samples.size(), slot.samples.data(), nullptr);
            cfg.sampleRate = Synth::SAMPLE_RATE;
            if (ma_audio_buffer_init(&cfg, &slot.buffers[v]) != MA_SUCCESS) return false;
            if (ma_sound_init_from_data_source(&engine, &slot.buffers[v],
                    MA_SOUND_FLAG_NO_SPATIALIZATION, nullptr, &slot.voices[v]) != MA_SUCCESS)
                return false;
        }
        return true;
    }

    // ===== Section C2: gibberish voices =====
    // Cartoon game characters often "talk" in fast nonsense syllables instead of words,
    // while the real words appear as text. Each syllable here is:
    //   a tiny noise burst (the consonant: "b", "t", "k"...)
    //   + a short buzzy tone that slides in pitch (the vowel: "ah", "oo", "eh"...)
    // The random pitch jumps give the funny, babbling voice.
    struct VoiceStyle {
        unsigned seed;       // different seed = different "sentence"
        float basePitch;     // Hz: higher = smaller/younger voice, lower = bigger voice
        float pitchRange;    // how many semitones the voice jumps around
        int   syllables;
        float syllableTime;  // seconds per syllable (smaller = faster talking)
        float volume;
    };

    unsigned rngState = 1;
    float random01() {
        rngState = rngState * 1664525u + 1013904223u;
        return (rngState >> 8) / 16777216.0f;   // 0 to 1
    }

    void makeBabble(Synth::Samples& s, const VoiceStyle& v) {
        rngState = v.seed;
        float t = 0.0f;
        for (int i = 0; i < v.syllables; i++) {
            float len = v.syllableTime * (0.7f + random01() * 0.6f);

            // Pitch: random jump around the base, rising a bit at the end like a question
            float semitones = (random01() * 2.0f - 0.6f) * v.pitchRange;
            if (i == v.syllables - 1) semitones += 3.0f;
            float pitch = v.basePitch * std::pow(2.0f, semitones / 12.0f);
            float glide = pitch * (0.85f + random01() * 0.35f);

            // Consonant: a very short burst of noise
            Synth::addNote(s, t, 0.018f, 0, 0, Wave::Noise, v.volume * 0.8f);
            // Vowel: buzzy base tone + brighter overtones ("formants")
            Synth::addSustained(s, t + 0.012f, len, pitch, glide, Wave::Square,   v.volume * 0.45f, 0.03f);
            Synth::addSustained(s, t + 0.012f, len, pitch * 2.0f, glide * 2.0f, Wave::Sine, v.volume * 0.55f, 0.03f);
            Synth::addSustained(s, t + 0.012f, len, pitch * 3.0f, glide * 2.9f, Wave::Triangle, v.volume * 0.30f, 0.03f);

            t += len + 0.025f;
            if (random01() < 0.22f) t += 0.07f;   // little pauses between "words"
        }
    }

    void makeVoices() {
        //                        seed  pitch range syl  time   vol
        makeBabble(effects[(int)Sfx::TalkHero].samples,        { 11, 210, 7,  12, 0.085f, 0.30f });
        makeBabble(effects[(int)Sfx::TalkHeroExcited].samples, { 23, 260, 10,  8, 0.075f, 0.32f });
        makeBabble(effects[(int)Sfx::TalkHeroShout].samples,   { 37, 230, 12, 10, 0.110f, 0.40f });
        makeBabble(effects[(int)Sfx::TalkDriver].samples,      { 51, 150, 8,  13, 0.080f, 0.32f });
        makeBabble(effects[(int)Sfx::TalkBig].samples,         { 77,  90, 6,  10, 0.130f, 0.40f });
    }

    // ===== Section C3: vehicle horns and noises =====
    void makeVehicleNoises() {
        // Tuk-tuk: squeaky little double beep
        auto& tuk = effects[(int)Sfx::HornTuktuk].samples;
        for (int i = 0; i < 2; i++) {
            Synth::addSustained(tuk, i * 0.22f, 0.16f, 1250, 1250, Wave::Square, 0.18f);
            Synth::addSustained(tuk, i * 0.22f, 0.16f, 1580, 1580, Wave::Square, 0.12f);
        }

        // Car: classic two-tone "honk honk" (two notes played together)
        auto& car = effects[(int)Sfx::HornCar].samples;
        for (int i = 0; i < 2; i++) {
            Synth::addSustained(car, i * 0.32f, 0.24f, 400, 400, Wave::Square, 0.16f);
            Synth::addSustained(car, i * 0.32f, 0.24f, 500, 500, Wave::Square, 0.14f);
        }

        // Microbus: a long honk, then a short musical tune horn (common on Cairo microbuses)
        auto& micro = effects[(int)Sfx::HornMicrobus].samples;
        Synth::addSustained(micro, 0.0f, 0.45f, 330, 330, Wave::Square, 0.15f);
        Synth::addSustained(micro, 0.0f, 0.45f, 415, 415, Wave::Square, 0.13f);
        const int tune[4] = { 76, 79, 76, 72 };
        for (int i = 0; i < 4; i++) {
            float f = Synth::noteFreq(tune[i]);
            Synth::addSustained(micro, 0.55f + i * 0.13f, 0.11f, f, f, Wave::Square, 0.14f);
        }

        // Bus: huge, deep, slightly sliding air horn
        auto& bus = effects[(int)Sfx::HornBus].samples;
        Synth::addSustained(bus, 0.0f, 1.1f, 175, 168, Wave::Square,   0.20f);
        Synth::addSustained(bus, 0.0f, 1.1f, 220, 212, Wave::Square,   0.17f);
        Synth::addSustained(bus, 0.0f, 1.1f, 110, 106, Wave::Triangle, 0.30f);

        // Car zooming past: engine tone that rises then falls (the "Doppler" effect) + road noise
        auto& zoom = effects[(int)Sfx::CarZoom].samples;
        Synth::addSustained(zoom, 0.0f, 0.45f, 140, 260, Wave::Square, 0.16f);
        Synth::addSustained(zoom, 0.45f, 0.45f, 230, 110, Wave::Square, 0.16f);
        Synth::addNote(zoom, 0.0f, 0.9f, 0, 0, Wave::Noise, 0.15f);

        // Tire screech: a high whine that wavers
        auto& screech = effects[(int)Sfx::Screech].samples;
        Synth::addSustained(screech, 0.0f, 0.45f, 2100, 1700, Wave::Triangle, 0.12f, 0.04f);
        Synth::addSustained(screech, 0.0f, 0.45f, 2900, 2500, Wave::Sine, 0.08f, 0.05f);
        Synth::addNote(screech, 0.0f, 0.45f, 0, 0, Wave::Noise, 0.10f);
    }

    // ===== Section C: design every sound effect =====
    void makeEffects() {
        auto& attack = effects[(int)Sfx::Attack].samples;      // "whoosh"
        Synth::addNote(attack, 0.0f, 0.14f, 900, 250, Wave::Noise, 0.35f);
        Synth::addNote(attack, 0.0f, 0.14f, 700, 200, Wave::Sine,  0.25f);

        auto& hit = effects[(int)Sfx::Hit].samples;            // "thud"
        Synth::addNote(hit, 0.0f, 0.15f, 180, 70, Wave::Square, 0.35f);
        Synth::addNote(hit, 0.0f, 0.05f, 0,   0,  Wave::Noise,  0.40f);

        auto& death = effects[(int)Sfx::Death].samples;        // falling tone
        Synth::addNote(death, 0.0f, 0.55f, 420, 55, Wave::Square, 0.30f);

        auto& coin = effects[(int)Sfx::Coin].samples;          // classic two-note "ding"
        Synth::addNote(coin, 0.00f, 0.08f, 988,  988,  Wave::Square, 0.25f);
        Synth::addNote(coin, 0.07f, 0.25f, 1319, 1319, Wave::Square, 0.25f);

        auto& lane = effects[(int)Sfx::LaneSelect].samples;    // short blip
        Synth::addNote(lane, 0.0f, 0.06f, 660, 660, Wave::Sine, 0.45f);

        auto& place = effects[(int)Sfx::Place].samples;        // rising "pop"
        Synth::addNote(place, 0.0f, 0.12f, 280, 560, Wave::Triangle, 0.55f);

        auto& click = effects[(int)Sfx::Click].samples;        // menu click
        Synth::addNote(click, 0.0f, 0.04f, 1000, 1000, Wave::Sine, 0.40f);

        auto& win = effects[(int)Sfx::Win].samples;            // rising notes (C E G C)
        int winNotes[4] = { 72, 76, 79, 84 };
        for (int i = 0; i < 4; i++) {
            float len = (i == 3) ? 0.6f : 0.15f;
            Synth::addNote(win, i * 0.13f, len, Synth::noteFreq(winNotes[i]),
                           Synth::noteFreq(winNotes[i]), Wave::Square, 0.22f);
        }

        auto& lose = effects[(int)Sfx::Lose].samples;          // sad falling notes
        int loseNotes[4] = { 67, 66, 65, 64 };
        for (int i = 0; i < 4; i++) {
            float len = (i == 3) ? 0.9f : 0.25f;
            Synth::addNote(lose, i * 0.25f, len, Synth::noteFreq(loseNotes[i]),
                           Synth::noteFreq(loseNotes[i]) * (i == 3 ? 0.94f : 1.0f),
                           Wave::Triangle, 0.45f);
        }

        makeVoices();
        makeVehicleNoises();
    }

    // ===== Section D: background music (an Egyptian-style loop) =====
    // Maqam Hijaz on D (D Eb F# G A Bb C) gives the Middle-Eastern sound,
    // and a darbuka "maqsum" rhythm (DUM tek . tek DUM . tek .) drives it.
    void makeMusic() {
        auto& s = music.samples;
        const float eighth = 60.0f / 112.0f / 2.0f;   // 112 beats per minute
        const int   bars   = 8;

        // Melody: pairs of {note, length in eighths}. 0 = rest. Each bar = 8 eighths.
        const int melody[][2] = {
            {62,2},{63,1},{66,1},{67,2},{66,2},     // bar 1
            {63,1},{62,1},{63,2},{62,4},            // bar 2
            {69,2},{67,1},{66,1},{67,2},{69,2},     // bar 3
            {70,1},{69,1},{67,2},{66,2},{63,1},{62,1}, // bar 4
            {67,2},{69,1},{70,1},{72,2},{70,2},     // bar 5
            {69,1},{67,1},{69,2},{67,2},{66,2},     // bar 6
            {67,1},{66,1},{63,2},{66,1},{63,1},{62,2}, // bar 7
            {62,6},{0,2}                            // bar 8
        };
        float t = 0.0f;
        for (const auto& n : melody) {
            float len = n[1] * eighth;
            if (n[0] != 0) {
                float f = Synth::noteFreq(n[0]);
                Synth::addNote(s, t, len * 0.95f, f, f, Wave::Square, 0.07f);
                Synth::addNote(s, t, len * 0.95f, f * 2, f * 2, Wave::Sine, 0.04f);
            }
            t += len;
        }

        // Bass: one low note per half bar, following the melody
        const int bassRoots[bars] = { 38, 38, 36, 38, 43, 43, 36, 38 };
        for (int bar = 0; bar < bars; bar++) {
            for (int half = 0; half < 2; half++) {
                float start = (bar * 8 + half * 4) * eighth;
                float f = Synth::noteFreq(bassRoots[bar]);
                Synth::addNote(s, start, eighth * 3.5f, f, f, Wave::Triangle, 0.28f);
            }
        }

        // Darbuka: 'D' = DUM (deep), 'T' = tek (sharp), '.' = silence
        const char* rhythm = "DT.TD.T.";
        for (int bar = 0; bar < bars; bar++) {
            for (int i = 0; i < 8; i++) {
                float start = (bar * 8 + i) * eighth;
                if (rhythm[i] == 'D') Synth::addNote(s, start, 0.20f, 120, 55, Wave::Sine, 0.55f);
                if (rhythm[i] == 'T') Synth::addNote(s, start, 0.05f, 0, 0, Wave::Noise, 0.18f);
            }
        }

        // Make the loop exactly 8 bars long so it repeats without a gap
        s.resize((size_t)(bars * 8 * eighth * Synth::SAMPLE_RATE), 0.0f);
    }
}

// ===== Section E: public functions =====
bool Audio::init() {
    if (ma_engine_init(nullptr, &engine) != MA_SUCCESS) {
        std::cerr << "Audio: no sound device found, the game will run silently\n";
        return false;
    }
    ready = true;

    makeEffects();
    makeMusic();

    for (auto& slot : effects) {
        if (!prepare(slot, VOICES)) { std::cerr << "Audio: could not prepare a sound\n"; }
    }
    if (prepare(music, 1)) {
        ma_sound_set_looping(&music.voices[0], MA_TRUE);
        ma_sound_set_volume(&music.voices[0], 0.6f);
    }
    applyVolume();
    return true;
}

void Audio::shutdown() {
    if (!ready) return;
    for (auto& slot : effects) {
        for (int v = 0; v < VOICES; v++) {
            ma_sound_uninit(&slot.voices[v]);
            ma_audio_buffer_uninit(&slot.buffers[v]);
        }
    }
    ma_sound_uninit(&music.voices[0]);
    ma_audio_buffer_uninit(&music.buffers[0]);
    ma_engine_uninit(&engine);
    ready = false;
}

void Audio::play(Sfx sound) {
    if (!ready || sound == Sfx::Count) return;
    SoundSlot& slot = effects[(int)sound];
    ma_sound& voice = slot.voices[slot.nextVoice];
    slot.nextVoice = (slot.nextVoice + 1) % VOICES;   // take turns between voices

    ma_sound_seek_to_pcm_frame(&voice, 0);   // rewind to the start
    ma_sound_start(&voice);
}

void Audio::startMusic() {
    if (!ready) return;
    if (!ma_sound_is_playing(&music.voices[0])) {
        ma_sound_seek_to_pcm_frame(&music.voices[0], 0);
        ma_sound_start(&music.voices[0]);
    }
}

void Audio::stopMusic() {
    if (ready) ma_sound_stop(&music.voices[0]);
}

void Audio::setMusicVolume(float amount) {
    if (ready) ma_sound_set_volume(&music.voices[0], 0.6f * amount);
}

void Audio::volumeUp() {
    masterVolume += 0.1f;
    if (masterVolume > 1.0f) masterVolume = 1.0f;
    muted = false;
    applyVolume();
}

void Audio::volumeDown() {
    masterVolume -= 0.1f;
    if (masterVolume < 0.0f) masterVolume = 0.0f;
    applyVolume();
}

void Audio::toggleMute() {
    muted = !muted;
    applyVolume();
}

float Audio::volume()  { return masterVolume; }
bool  Audio::isMuted() { return muted; }
