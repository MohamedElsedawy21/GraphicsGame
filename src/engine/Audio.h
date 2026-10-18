#pragma once

// Every sound effect in the game. Teammates play one with: Audio::play(Sfx::Hit);
enum class Sfx {
    Attack,      // a player or enemy attacks (slipper swing, stone throw)
    Hit,         // something takes damage
    Death,       // a player or enemy is defeated
    Coin,        // a coin is collected
    LaneSelect,  // a lane is selected or deselected
    Place,       // a player is placed on the grid
    Click,       // a menu button or card is clicked
    Win,         // the player won
    Lose,        // the player lost

    // ---- Cutscene voices: gibberish "babble" talking, like cartoon game characters ----
    TalkHero,         // Am Sayed, normal
    TalkHeroExcited,  // Am Sayed, surprised / annoyed
    TalkHeroShout,    // Am Sayed, "This is war!"
    TalkDriver,       // tuk-tuk / microbus drivers
    TalkBig,          // the big bus driver (deep voice)

    // ---- Vehicle noises ----
    HornTuktuk,       // squeaky "beep beep"
    HornCar,          // normal car "honk honk"
    HornMicrobus,     // long musical horn
    HornBus,          // huge deep air horn
    CarZoom,          // a car speeding past
    Screech,          // tires screeching to a stop

    Count        // not a sound: just the number of sounds above
};

namespace Audio {
    bool init();       // makes all sounds; returns false if there is no audio device
    void shutdown();

    void play(Sfx sound);
    void startMusic();
    void stopMusic();
    void setMusicVolume(float amount);   // 1.0 = normal; lower it so voices are heard

    // Sound settings menu
    void  volumeUp();
    void  volumeDown();
    void  toggleMute();
    float volume();    // 0.0 to 1.0
    bool  isMuted();
}
