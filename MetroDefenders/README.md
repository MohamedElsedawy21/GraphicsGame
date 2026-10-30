# Metro Defenders – The Egyptian Street

Pedestrians defend their Cairo metro station from tuk-tuks, microbuses and taxis.
The project is built on the course template (OpenGL 4.3 core, GLFW, GLAD, GLM).

## How to run
1. Open `Template/Template.sln` in Visual Studio.
2. Select **x64** and **Debug** (or Release), then press **F5**.
3. Keep the `sounds/` folder next to the source files. Visual Studio runs the game from the project folder, so the game finds the sounds automatically.
   - To build without sound, set `#define USE_AUDIO 0` in `audio.h`.
   - If VS asks to retarget the toolset (v142 → your version), accept.

## How to run with CMake
From the `MetroDefenders` folder (the one with `CMakeLists.txt`):
```
cmake -S . -B build
cmake --build build
.\build\game.exe
```
- Visual Studio compiler: uses the template's `glfw3.lib`.
- MinGW g++: GLFW is downloaded and built automatically the first time (needs internet once).
- The sounds are copied into `build\sounds` automatically.

## How to run in VS Code
Open the **`Template`** folder (the one that contains `main.cpp`) in VS Code, then press **Ctrl+Shift+B**.
- **MSVC (recommended, works with the template's glfw3.lib):** install "Visual Studio Build Tools" (Desktop development with C++). Open **"x64 Native Tools Command Prompt for VS"** from the Start menu, `cd` to the `Template` folder and type `code .`. Then Ctrl+Shift+B → *Build & Run (MSVC)*.
- **MinGW g++:** the template's glfw3.lib does not work with g++. Download the 64-bit Windows binaries from glfw.org, copy `lib-mingw-w64/libglfw3.a` into `Template/lib/mingw/`, then Ctrl+Shift+B → *Build & Run (MinGW g++)*.

## Controls
| Action | Input |
|---|---|
| Highlight a lane | Hover the mouse over it |
| Select / deselect a lane (several allowed) | Left click |
| Place a player | Click an unlocked card while lanes are selected (one per selected lane), **or** click a card and then click a lane |
| Cancel a chosen card | Right click |
| Jump (avoids LOW attacks) | W / Up arrow (players in the selected lanes) |
| Duck (avoids HIGH attacks) | S / Down arrow |
| Pause | Esc |
| Skip cutscene | Enter / Space |

## Characters
| Role | Character | Why it fits |
|---|---|---|
| Home base | Metro station: four stair entrances, red "M" signs, red metro band | What the pedestrians protect |
| Melee player (10 coins, 5 HP) | Vendor in a galabeya swinging a *nabboot* (traditional stick) | Stick swing is animated in his hand |
| Ranged player (40 coins, 5 HP) | Kid with a slingshot | Pulls back, fires a pebble that shatters on impact |
| Defense player (60 coins, 7 HP) | Traffic officer with a red/white metal barrier | Stationary, never attacks. Blocks vehicles and absorbs every attack for the home base and the players behind him. Durability is 7 **hits**: every attack counts as one hit, even the taxi's 2-damage ram |
| Easy enemy (5 HP, 1 dmg) | Black tuk-tuk | The driver throws tires at head height → **duck** |
| Medium enemy (5 HP, 1 dmg) | Plain white microbus | Gravel rolls along the ground → **jump** |
| Hard enemy (7 HP, 2 dmg) | Black-and-white Cairo taxi | Races in and rams at close range |
| Collectible | Egyptian 1-pound coin | Dropped by defeated vehicles; picked up when a player walks over it |

**Coins:** you start with 30, enough for 3 vendors, so only the vendor card is unlocked. The melee unlock requirement is 20, so ranged = 2 × 20 = 40 and defense = 3 × 20 = 60. This matches the 10/40/60 snippet in the brief. Cards lock and unlock automatically as your coins go up or down.

**Waves:**
- Wave 1: 3 tuk-tuks.
- Wave 2: 5 enemies (tuk-tuks and microbuses).
- Wave 3: 10 enemies (all three types).

Lanes are random and the mix is shuffled every game.

**Environment (more than 5 elements):**
- A kiosk (*koshk*), a ful cart, a bench, trash bins and a street cat
- Manholes, apartment blocks with satellite dishes, and the pyramids in the haze

## Cutscene (17.5 s)
1. **Morning in Cairo.** Commuters walk to the metro.
2. **The vehicles arrive.** The microbus driver says: "This square is our new microbus parking lot!"
3. **The vendor answers:** "No way! This station is our only way to work!" The kid's pebble bounces off the tuk-tuk.
4. **The taxi charges.** The traffic officer jumps in with his barrier: "Stop! Nobody touches our metro station!"
5. **"DEFEND THE METRO STATION!"** The game starts.

## Files (suggested split for a team of 4)
| File | Contents | Owner |
|---|---|---|
| `renderer.cpp/.h` | Shader, one global VBO/VAO/EBO with all basic shapes, matrix stack, stroke font, `pointInTriangle` | Member 1 |
| `models.cpp/.h` | All characters, vehicles, the station and the environment | Member 2 |
| `game.cpp/.h` | Lanes, waves, combat, jump/duck, defense protection, coins, cards, HUD | Member 3 |
| `cutscene.cpp`, `menus.cpp`, `main.cpp`, `audio.cpp` | Cutscene, the three menus plus sound settings, callbacks and game loop, audio bonus | Member 4 |

## Techniques used (all from the labs)
- VAO/VBO/EBO, `glVertexAttribPointer` with position and colour attributes (Lab 1/2).
- One global VBO, with each shape drawn with `glDrawArrays(mode, first, count)` (Lab 3). Circles use `GL_TRIANGLE_FAN`; the quad uses the EBO.
- `u_Transform` built with `glm::translate / rotate / scale` (Lab 2). The world uses pixels: `translate(-1,-1) * scale(2/W, 2/H)`.
- Fragment shader with `u_Tint` (Lab 2) and `mix(...)` highlight (Lab 3). This gives the red damage tint, the grayscale locked cards and the fade.
- `glfwSetKeyCallback`, `glfwSetMouseButtonCallback`, `glfwSetCursorPosCallback` (Lab 3).
- `pointInTriangle` collision (Lab 3) for buttons, cards and lanes (each rectangle = 2 triangles).
- `deltaTime` from `glfwGetTime()` (Lab 2) for all movement and animation.

Text (menus, subtitles, numbers) is drawn with our own line-segment font made of thin quads, so no font library is needed.

## Bonus audio
[miniaudio](https://miniaud.io) (single header, public domain) plays:
- Looping background music in *maqam hijaz*, with oud plucks and a darbuka rhythm
- Sound effects for lane selection, player placement, attacks and hits, deaths, coin pickup, honks, jump/duck, and win/lose

All sounds were synthesized for this project, so there are no copyright issues. Volume up, volume down and mute are under **Main Menu → Sound Settings**.
