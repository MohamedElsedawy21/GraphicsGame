#pragma once

// What the player chose in a menu this frame
enum class MenuAction {
    None,
    Start, OpenSound, Exit,               // main menu
    VolumeUp, VolumeDown, ToggleMute, Back, // sound settings
    Continue, Restart, MainMenu           // pause and win/lose menus
};

// Each menu has two halves: handle...() reads the mouse and returns the choice,
// draw...() paints it. The Game calls handle once per frame and draw every frame.
namespace Menus {
    MenuAction handleMain();
    void       drawMain(float time);

    MenuAction handleSound();
    void       drawSound(float time);

    MenuAction handlePause();
    void       drawPause();

    MenuAction handleEnd();
    void       drawEnd(bool won, float time);
}
