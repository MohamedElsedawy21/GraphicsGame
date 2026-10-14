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
    Count        // not a sound: just the number of sounds above
};

namespace Audio {
    bool init();       // makes all sounds; returns false if there is no audio device
    void shutdown();

    void play(Sfx sound);
    void startMusic();
    void stopMusic();

    // Sound settings menu
    void  volumeUp();
    void  volumeDown();
    void  toggleMute();
    float volume();    // 0.0 to 1.0
    bool  isMuted();
}
