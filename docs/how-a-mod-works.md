# How a GTA San Andreas mod works: from code to game

This document explains, step by step, what actually happens between writing C++ in this repo and seeing CJ fly on screen. It uses two real features of **ArmageddonPlus** as worked examples:

- **The HUD and power menu.** How text and boxes get drawn on top of the game, and how keys are read.
- **The Flight power.** How a C++ object ends up moving the game's real player character through the game's real physics.

It's written for someone who knows programming, web development for example, but has never done game modding. Every claim about plugin-sdk here was checked against its source in `C:\dev\plugin-sdk`.

## Contents
1. [The big picture](#1-the-big-picture)
2. [The game as a program](#2-the-game-as-a-program)
3. [The modding layers](#3-the-modding-layers)
4. [From source code to a file the game loads](#4-from-source-code-to-a-file-the-game-loads)
5. [Game startup: how our code gets inside the game](#5-game-startup-how-our-code-gets-inside-the-game)
6. [How plugin-sdk talks to the game](#6-how-plugin-sdk-talks-to-the-game)
7. [One frame of GTA San Andreas](#7-one-frame-of-gta-san-andreas)
8. [Worked example 1: the HUD and the power menu](#8-worked-example-1-the-hud-and-the-power-menu)
9. [Worked example 2: the Flight power](#9-worked-example-2-the-flight-power)
10. [The same idea in CLEO](#10-the-same-idea-in-cleo)
11. [Debugging, crashes and safety](#11-debugging-crashes-and-safety)
12. [Glossary](#12-glossary)

---

## 1. The big picture

GTA San Andreas was released in 2005 with no official modding support. All PC modding is built on one fact:

> **The game is a normal Windows program.** If you can get your own code to run *inside its process*, you can read and change anything in its memory and call any of its functions.

Everything else is tooling to make that practical and safe:

```
 your C++ (src/*.cpp)
      │  MSBuild + MSVC, linked with plugin-sdk's Plugin.lib
      ▼
 ArmageddonPlus.asi                    a DLL with a different extension
      │  scripts/deploy.ps1
      ▼
 modloader\ArmageddonPlus\             in the game folder
      │  gta_sa.exe starts → ASI loader → Modloader → LoadLibrary(our .asi)
      ▼
 our code runs inside gta_sa.exe
      │  plugin-sdk: hooks the game loop, wraps game functions/variables/classes
      ▼
 every frame: read keys → change engine objects (CJ's velocity...) → draw HUD
      │  the game's own physics, animation and renderer do the rest
      ▼
 CJ flies on screen
```

## 2. The game as a program

### 2.1 What `gta_sa.exe` is
- A **32-bit (x86) Windows executable**, written in C++ by Rockstar North. So our plugin must also be 32-bit: that's the `Win32` platform in the `.vcxproj`.
- It uses the **RenderWare** engine (by Criterion) for rendering, models (`.dff`), textures (`.txd`) and animations. On top of RenderWare, Rockstar wrote the game logic: `CPed`, `CVehicle`, `CWorld`, `CPhysical`, `CCamera`, `CHud`... Thanks to the community's reverse engineering, we know these classes' names, fields and functions.
- Missions and world logic run in a **script virtual machine**: `data\script\main.scm` is bytecode. Each instruction is an **opcode**, for example `00A5` = create a car, or `01B4` = set player control. CLEO adds more opcodes and lets you run extra scripts (`.cs`).

### 2.2 Why the exact version (1.0 US) matters
Inside the exe, every function and every global variable lives at a **fixed memory address**. Examples from plugin-sdk:

| Thing | Address in 1.0 US | What it is |
|---|---|---|
| `CFont::PrintString` | `0x71A700` | function that queues text for drawing |
| `CSprite2d::DrawRect` | `0x727B60` | function that draws a filled rectangle |
| `FindPlayerPed` | `0x56E210` | function returning the player's `CPlayerPed*` |
| `CTimer::ms_fTimeStep` | `0xB7CB5C` | global float: length of this frame in 1/50 s units |
| `CTimer::ms_fTimeScale` | `0xB7CB64` | global float: game speed (Bullet Time writes it) |

A different build of the game (Steam 3.0, 1.01...) is compiled differently, so every address moves. That's why [setup step 2](setup.md#2-downgrade-the-copy-to-10-us) downgrades the copy to **1.0 US**: plugin-sdk (with `PLUGIN_SGV_10US`), CLEO and nearly every mod target those addresses.

### 2.3 Units and coordinates
- **World:** 1 unit = 1 meter. X = east, Y = north, **Z = up**.
- **Time:** the game measures time in **steps of 1/50 s**. `CTimer::ms_fTimeStep` ≈ 1.0 at 50 FPS and ≈ 0.83 at 60 FPS.
- **Velocity:** `CPhysical::m_vecMoveSpeed` is **meters per step**, not per second. 10 m/s = `10 / 50` = `0.2`. Every power that sets a velocity divides by 50.
- **Heading:** radians, 0 = facing north (+Y), increasing counter-clockwise.

---

## 3. The modding layers

What's installed in `C:\Games\GTA-SA-Modding`, from the bottom up:

```
┌──────────────────────────────────────────────────────────────────────┐
│ CLEO scripts (.cs)       hello, vehicle_spawner, console             │  run in the script VM
├──────────────────────────────────────────────────────────────────────┤
│ .asi plugins             CLEO.asi  SilentPatchSA.asi  modloader.asi  │  native code in the
│                          └─ via Modloader: ArmageddonPlus.asi        │  game process
├──────────────────────────────────────────────────────────────────────┤
│ ASI loader               vorbisFile.dll (Ultimate ASI Loader)        │  loads every .asi
├──────────────────────────────────────────────────────────────────────┤
│ gta_sa.exe 1.0 US        + RenderWare, main.scm, data files          │  the game
└──────────────────────────────────────────────────────────────────────┘
```

- **ASI loader.** The game normally loads `vorbisFile.dll` (an audio library) at startup. The Ultimate ASI Loader *pretends to be* that DLL. It forwards the real audio functions, and before the game starts, it calls Windows' `LoadLibrary` on every `*.asi` file it finds. An `.asi` is just a DLL; the different extension only marks it as a plugin.
- **CLEO** (`CLEO.asi`) extends the script VM with new opcodes and runs `.cs` files.
- **SilentPatch** fixes original bugs (frame-rate issues, crashes on modern Windows...).
- **Modloader** (`modloader.asi`) loads files from `modloader\<anything>\` *as if* they were in the game folders. That's why we never copy into the game's own folders: `deploy.ps1` writes `modloader\ArmageddonPlus\ArmageddonPlus.asi`, and deleting that folder uninstalls the mod.

---

## 4. From source code to a file the game loads

### 4.1 The project
[plugins/ArmageddonPlus/ArmageddonPlus.vcxproj](../plugins/ArmageddonPlus/ArmageddonPlus.vcxproj) is an MSBuild project, the C++ equivalent of a `package.json` + build config:

| Setting | Value | Why |
|---|---|---|
| `ConfigurationType` | `DynamicLibrary` | build a DLL |
| `TargetExt` | `.asi` | so the ASI loader picks it up |
| `Platform` | `Win32` | the game is 32-bit |
| `PlatformToolset` | `v143` | the VS 2022 compiler |
| defines | `GTASA`, `PLUGIN_SGV_10US`, `RW`... | tell plugin-sdk's headers which game and version to target |
| include dirs | `$(PLUGIN_SDK_DIR)\plugin_sa\game_sa`, `\shared`... | the reverse-engineered game headers (`CPed.h`, `CFont.h`...) |
| link | `Plugin.lib` from `$(PLUGIN_SDK_DIR)\output\lib` | plugin-sdk's compiled wrappers |
| `RuntimeLibrary` | `MultiThreaded` (static) | the C++ runtime is baked into the `.asi`, so no extra DLLs are needed |

### 4.2 The pipeline
```
scripts/build.ps1
  └─ finds MSBuild with vswhere.exe
  └─ msbuild ArmageddonPlus.vcxproj /p:Configuration=Release /p:Platform=Win32
       ├─ cl.exe compiles src/**/*.cpp  → obj/Release/*.obj
       └─ link.exe links *.obj + Plugin.lib → bin/Release/ArmageddonPlus.asi
scripts/deploy.ps1
  └─ copies bin/Release/ArmageddonPlus.asi + config/ArmageddonPlus.ini
     → C:\Games\GTA-SA-Modding\modloader\ArmageddonPlus\
scripts/run.ps1
  └─ build + deploy + Start-Process gta_sa.exe
```
The game must be closed when deploying, because Windows locks a loaded DLL.

---

## 5. Game startup: how our code gets inside the game

What happens when you double-click `gta_sa.exe`:

1. **Windows loads `gta_sa.exe`** and the DLLs it imports. One of them is `vorbisFile.dll`, which is really the ASI loader.
2. **The ASI loader runs.** It loads every `.asi` in the game folder: `CLEO.asi`, `SilentPatchSA.asi`, `modloader.asi`.
3. **Modloader starts up** and scans `modloader\*\`. It finds `ArmageddonPlus\ArmageddonPlus.asi` and loads it with `LoadLibrary`.
4. **Windows maps our DLL into the game's memory** and runs its initialization. As part of that, the C++ runtime constructs every **global object**. We have exactly one that matters, at the bottom of [main.cpp](../plugins/ArmageddonPlus/src/main.cpp):
   ```cpp
   class ArmageddonPlus {
   public:
       ArmageddonPlus() { ... }
   } g_armageddonPlus; // global instance: its constructor is our "main"
   ```
5. **Our "main" (the constructor) runs.** The game hasn't started its loop yet; we're still at startup:
   ```cpp
   Log::Init(paths::GetGameDirRelativePathA("ArmageddonPlus.log"));
   g_powers.Register(std::make_unique<Flight>());       // create the powers
   ...
   config.Load(paths::GetPluginDirRelativePathA("ArmageddonPlus.ini"));
   g_powers.LoadConfig(config);                         // read [Power.Flight] etc.
   g_menu.LoadConfig(config);

   Events::gameProcessEvent += [] { ... };              // "call me every frame"
   Events::drawHudEvent += [] { ... };                  // "call me while drawing the HUD"
   ```
   Those `+=` lines don't just store a callback. The first subscription **patches the game's machine code**; see [6.4](#64-events-patching-the-game-loop).
6. **The constructor returns**, and the ASI loader moves on to the next plugin. Then the game boots normally: intro movies, main menu, loading a save. **Our code doesn't run again until the game reaches the patched spots.**

---

## 6. How plugin-sdk talks to the game

plugin-sdk is a big set of headers (`C:\dev\plugin-sdk\plugin_sa\game_sa\*.h`) plus small `.cpp` wrappers. It provides four mechanisms.

### 6.1 Calling game functions: cast an address
`CFont::PrintString` in plugin-sdk ([CFont.cpp](https://github.com/DK22Pac/plugin-sdk/blob/15f15b6/plugin_sa/game_sa/CFont.cpp)) is literally:
```cpp
void CFont::PrintString(float x, float y, const char *text)
{
    ((void (__cdecl *)(float, float, const char *))0x71A700)(x, y, text);
}
```
Read it inside out: take the number `0x71A700`, treat it as a pointer to a function with this signature and calling convention, and call it. The code that runs is **Rockstar's**, inside `gta_sa.exe`. Methods use the `__thiscall` convention (the object pointer goes in a register):
```cpp
void CPlaceable::SetPosn(CVector const& posn)
{
    ((void (__thiscall *)(CPlaceable *, CVector const&))0x4241C0)(this, posn);
}
```

### 6.2 Reading and writing game variables: references to addresses
```cpp
float& CTimer::ms_fTimeStep  = *(float*)0xB7CB5C;
float& CTimer::ms_fTimeScale = *(float*)0xB7CB64;
```
`CTimer::ms_fTimeScale = 0.35f;` writes directly into the game's memory. That one line *is* Bullet Time.

### 6.3 Game objects: matching memory layout
`CPed`, `CVehicle` and the other classes are declared with **exactly** the same field order and sizes as Rockstar's originals, so a pointer the game gives us can be used as a normal C++ object. The headers check this at compile time:
```cpp
VALIDATE_OFFSET(CPhysical, m_vecMoveSpeed, 0x44);  // must be 0x44 bytes into the object
VALIDATE_SIZE(CPed, 0x79C);                        // a ped is exactly 0x79C bytes
```
So when Flight writes `player->m_vecMoveSpeed = ...`, the compiler emits "write 12 bytes at `player + 0x44`". That's the same memory the game's physics reads a moment later.

The class hierarchy mirrors the game's:
```
CPlaceable            position + rotation matrix        (GetPosition, SetPosn, SetHeading)
└─ CEntity            model, type, flags (bIsStatic...)
   └─ CPhysical       velocity, mass, gravity, proofs   (m_vecMoveSpeed, bApplyGravity, bCollisionProof)
      ├─ CPed         health, tasks, weapons, animations
      │  └─ CPlayerPed   CJ
      ├─ CVehicle     cars, bikes, planes...
      └─ CObject      props
```

### 6.4 Events: patching the game loop
The game's main loop has a line that, in C++ terms, reads `CGame::Process();`. Compiled, it's a 5-byte x86 `CALL` instruction at address **`0x53E981`**. plugin-sdk's [Events.h](https://github.com/DK22Pac/plugin-sdk/blob/15f15b6/shared/Events.h) declares:
```cpp
static inline CdeclEvent<AddressList<0x53E981, H_CALL>, PRIORITY_AFTER, ArgPickNone, void()> gameProcessEvent;
static inline CdeclEvent<AddressList<0x53E4FF, H_CALL>, PRIORITY_AFTER, ArgPickNone, void()> drawHudEvent;
```
The first `+=` triggers `Patch()` in [EventList.h](https://github.com/DK22Pac/plugin-sdk/blob/15f15b6/shared/EventList.h). Using the *injector* library, it **rewrites the `CALL` at `0x53E981` to call a plugin-sdk stub instead**. The stub does:
```
run every "before" callback
call the original target (CGame::Process: the game updates the world)
run every "after" callback       ← our lambda: Input::Update(), menu, powers
return to the game
```
So from then on, every frame, right after the game finishes updating the world, our code runs. `drawHudEvent` works the same way at `0x53E4FF`, the spot where the game draws its HUD.

This is called **hooking**. It's the central technique of native game modding: find the call you care about, redirect it, and do your work before or after it.

### 6.5 Running script opcodes from C++
Some things are easiest done exactly the way missions do them. `Command<Commands::SET_PLAYER_CONTROL>(0, false)` ([ScriptCommands.h](https://github.com/DK22Pac/plugin-sdk/blob/15f15b6/shared/extensions/ScriptCommands.h)):
1. creates a fake, empty `CRunningScript` (a script "thread" object) named `plg-sdk`;
2. writes the opcode number and the arguments into a small bytecode buffer, encoded like `main.scm`;
3. asks the game's script VM to execute that one instruction.

It's the same code path as the CLEO line `set_player_control $player1 false`. The menu and Flight use it for player control and animations.

---

## 7. One frame of GTA San Andreas

At 60 FPS this whole cycle happens about every 16 ms. Our two hooks are marked ★.

```
Idle() — the game's frame function
│
├─ 1. CGame::Process()                                   ← the CALL at 0x53E981
│     ├─ CPad::UpdatePads()       keyboard/mouse/gamepad → pad state (walk axes,
│     │                           buttons). Our menu's SET_PLAYER_CONTROL(false)
│     │                           makes the pad report "nothing pressed" to CJ.
│     ├─ CTheScripts::Process()   main.scm + CLEO scripts run (console, spawner...)
│     ├─ CWorld::Process()        for every ped/vehicle/object:
│     │     ├─ ProcessControl()   AI tasks, animations, intentions
│     │     │     CPhysical: apply gravity (if bApplyGravity), forces, air resistance
│     │     ├─ ProcessCollision() move by m_vecMoveSpeed × timeStep, resolve hits
│     │     └─ animation update   advance anims by timeStep × anim speed
│     ├─ CCamera::Process()       camera follows CJ, mouse look
│     └─ fire, explosions, weather, streaming (loading models/anims from disk)
│
├─ ★ gameProcessEvent: OUR LOGIC
│     Input::Update()           GetKeyState for 255 keys → "just pressed" edges
│     Menu::Update(dt)          F10, arrows, Enter
│     PowerManager::Update()    each active power's Update(): Flight writes
│                               CJ's m_vecMoveSpeed here; it takes effect in step 1
│                               of the NEXT frame
│
├─ 2. Render the 3D world: RenderScene()
│     RenderWare draws buildings, peds, cars, effects (fire, explosions)
│
├─ 3. Draw the HUD                                        ← the CALL at 0x53E4FF
│     radar, health, money, weapon icon, help boxes
│
├─ ★ drawHudEvent: OUR DRAWING
│     powers' Draw() (Bullet Time tint, lightning bolt), watermark,
│     power list or menu
│
├─ 4. CFont::DrawFonts()   flushes the queued text (ours included)
└─ 5. Present: the finished frame goes to the monitor
```

Two consequences worth knowing:
- **Everything is single-threaded.** Our hooks run *between* the game's steps, so we never touch an object while the game is halfway through updating it. That's why plain field writes are safe.
- **We're always one step "late".** We react after the game processed the frame, and our changes apply on the next. At 60 FPS that's invisible.

---

## 8. Worked example 1: the HUD and the power menu

Files: [core/Input.cpp](../plugins/ArmageddonPlus/src/core/Input.cpp), [core/Menu.cpp](../plugins/ArmageddonPlus/src/core/Menu.cpp), [core/PowerManager.cpp](../plugins/ArmageddonPlus/src/core/PowerManager.cpp), [main.cpp](../plugins/ArmageddonPlus/src/main.cpp).

### 8.1 Reading the keyboard
plugin-sdk's `KeyPressed` is a thin wrapper around a Windows API:
```cpp
bool plugin::KeyPressed(unsigned int keyCode)
{
    return (GetKeyState(keyCode) & 0x8000) != 0;   // high bit = key is down right now
}
```
That only tells us "is it down", but a menu needs "was it *just* pressed", or holding Down would scroll 60 rows per second. [Input.cpp](../plugins/ArmageddonPlus/src/core/Input.cpp) keeps two snapshots:
```cpp
void Update() {                      // once per frame, first thing in gameProcessEvent
    for (int vk = 1; vk < 256; ++vk) {
        g_before[vk] = g_now[vk];          // last frame
        g_now[vk] = plugin::KeyPressed(vk); // this frame
    }
}
bool JustPressed(int vk) { return g_now[vk] && !g_before[vk]; }   // the "edge"
```
Sampling once per frame means every part of the plugin sees the same edges in the same frame.

Mouse buttons are "keys" too (`VK_LBUTTON`, `VK_RBUTTON`), which is how Telekinesis and Force read clicks.

### 8.2 Opening the menu (F10)
`Menu::Update` runs every frame, **before** the powers:
```cpp
if (Input::JustPressed(m_key)) {      // m_key = VK_F10 from [Menu] Key=F10
    if (m_open) Close(); else Open();
    return;
}
```
`Open()` does three things:
```cpp
m_open = true;
Command<Commands::SET_PLAYER_CONTROL>(0, false);   // 1. freeze CJ (opcode 01B4)
m_powers.SetHotkeysEnabled(false);                 // 2. pause power hotkeys
Sound(AE_FRONTEND_SELECT);                         // 3. the game's own menu "blip"
```
1. **Freezing CJ.** While the menu is open, ↑↓←→ and Enter would otherwise walk him around and get him into cars. `SET_PLAYER_CONTROL(false)` makes the game's pad return "nothing pressed" to CJ's controls. The game still runs, and our `Input` (which reads Windows directly, not the pad) still sees the keys.
   *Why not just set the flag directly?* We tried writing `CPad::bPlayerSafe` ourselves first. Then, while debugging a "CJ stays frozen" bug, the log showed the flag values were fine, and the real culprit was the **F1** key, which makes GTA SA stop reading the keyboard. The script command is still the more robust choice: it's exactly what missions use. See [troubleshooting](troubleshooting.md).
2. **Pausing hotkeys.** If a power had an optional hotkey on an arrow key, it would fire while you navigate. `PowerManager` skips hotkeys while this flag is off.
3. **Sound.** `AudioEngine.ReportFrontendAudioEvent(AE_FRONTEND_SELECT, 0.0f, 1.0f)` plays the same sound as the pause menu.

### 8.3 Navigating: key repeat
Holding ↓ should scroll: one step immediately, then after 0.4 s one step every 0.1 s. `Menu::Repeat` tracks how long the key has been held (`heldTime += dt`) and returns `true` when the held time crosses another multiple of the repeat rate:
```cpp
if (Repeat(VK_DOWN, dt, m_heldDown)) {
    m_selected = (m_selected + 1) % count;   // wrap around
    Sound(AE_FRONTEND_HIGHLIGHT);
}
if (Repeat(VK_RIGHT, dt, m_heldRight)) ChangeValue(+1);   // level up
if (Input::JustPressed(VK_RETURN)) Activate();            // toggle / fire
```
`ChangeValue(+1)` on Flight at level 1 calls `m_powers.SetLevel(flight, player, 2)`. What that triggers is described in [9.1](#91-turning-it-on).

### 8.4 Drawing: the HUD hook
In [main.cpp](../plugins/ArmageddonPlus/src/main.cpp):
```cpp
Events::drawHudEvent += [] {
    g_powers.DrawPowers();     // screen effects first (Bullet Time tint...)
    DrawWatermark();           // "ArmageddonPlus", top-right
    if (!g_menu.IsOpen())
        g_powers.DrawOverlay(); // the green/grey power list
    g_menu.Draw();             // the menu, on top of everything
};
```
This runs right after the game drew its own HUD (radar, health...), so our drawing ends up on top. Order matters: things drawn later cover things drawn earlier.

### 8.5 Screen coordinates
The game renders at your real resolution, e.g. 1920×1080. To make layouts work at any resolution, plugin-sdk's `SCREEN_COORD` scales a value designed for a **1080-pixel-tall** screen:
```cpp
float plugin::screen::GetCoord(float a) {
    float base = (width > height) ? height : width;   // the shorter side, e.g. 1080
    return (int)(a * base / 1080.0f);                 // gScreenResolution default = 1080
}
```
- `SCREEN_COORD(30)` = 30 px on a 1080p screen, 40 px on 1440p.
- `SCREEN_COORD_RIGHT(20)` = `screenWidth - SCREEN_COORD(20)`: 20 units from the right edge, used for the watermark.
- `SCREEN_MULTIPLIER(x)` is the same scale factor without rounding, used for font scales.

### 8.6 Drawing a box: `CSprite2d::DrawRect`
```cpp
void Rect(float x1, float y1, float x2, float y2, CRGBA color) {
    CSprite2d::DrawRect(CRect(SCREEN_COORD(x1), SCREEN_COORD(y1),
                              SCREEN_COORD(x2), SCREEN_COORD(y2)), color);
}
Rect(kLeft, kTop, kLeft + kWidth, bottom, CRGBA(0, 0, 0, 190));   // translucent black panel
```
`CSprite2d::DrawRect` is the game's function at `0x727B60` (the same one that draws its own menu backgrounds). It builds 4 vertices and draws them immediately as two triangles with RenderWare's 2D immediate mode. `CRGBA(r, g, b, a)`: an alpha of 190/255 lets the game show through.

The selected row is just another rectangle (`CRGBA(255, 255, 255, 40)`), drawn before that row's text.

### 8.7 Drawing text: `CFont`
`CFont` is a **state machine**, like an old-school drawing API. You set the style, then print:
```cpp
CFont::SetFontStyle(FONT_SUBTITLES);                 // the game's subtitle font
CFont::SetOrientation(ALIGN_LEFT);
CFont::SetProportional(true);                        // variable-width letters
CFont::SetScale(SCREEN_MULTIPLIER(0.38f), SCREEN_MULTIPLIER(0.9f));
CFont::SetColor(CRGBA(255, 200, 60, 255));           // selected row = orange
CFont::SetDropShadowPosition(1);
CFont::SetDropColor(CRGBA(0, 0, 0, 255));
CFont::PrintString(SCREEN_COORD(kLeft + kPadding), SCREEN_COORD(y), "Flight");
```
- **The state persists between calls, including into the game's own code,** so we set *everything* before each print. That's why `SetupText()` exists.
- `PrintString` doesn't draw pixels immediately. The game **queues** the text and renders the whole batch when it flushes fonts later in the frame (`CFont::DrawFonts`, `0x71A210`). The order is still respected.
- Text can contain the game's color codes, e.g. `~y~` (yellow) and `~n~` (new line).
- The title uses `FONT_PRICEDOWN`, the GTA logo font, which is why it looks "official".

### 8.8 The power list overlay
`PowerManager::DrawOverlay` loops over the registered powers, picks green for active ones and grey for inactive ones, and prints `Super Speed 2/3` or `Flight`, one line every `SCREEN_COORD(16)` pixels. New powers appear automatically because it just iterates `m_powers`.

### 8.9 Summary: one key press, end to end
```
you press ↓
 → Windows updates its key state
 → (frame N) CGame::Process runs; CJ ignores it (controls are off)
 → ★ gameProcessEvent: Input::Update sees VK_DOWN go up→down → JustPressed
 → Menu::Update → Repeat(VK_DOWN) = true → m_selected++ → highlight sound
 → game renders 3D, then the HUD
 → ★ drawHudEvent: Menu::Draw → rectangle at the new m_selected + text
 → CFont flushes the text → frame shown: the highlight moved
```

---

## 9. Worked example 2: the Flight power

Files: [powers/Flight.h](../plugins/ArmageddonPlus/src/powers/Flight.h), [powers/Flight.cpp](../plugins/ArmageddonPlus/src/powers/Flight.cpp), [core/Power.h](../plugins/ArmageddonPlus/src/core/Power.h), [core/PowerManager.cpp](../plugins/ArmageddonPlus/src/core/PowerManager.cpp).

### 9.0 The idea
GTA SA has no "flying" for people. But it has a general **physics system** (`CPhysical`) used by every ped, car and object:
- every frame, if `bApplyGravity` is set, gravity is added to the velocity;
- the object is moved by `m_vecMoveSpeed × timeStep`, **and collisions are resolved**.

So the plan is:
1. turn CJ's gravity **off**;
2. every frame, **write the velocity we want** into `m_vecMoveSpeed`;
3. let the game's own physics move him, so he still collides with buildings;
4. make him **look** like he's flying with borrowed animations.

### 9.1 Turning it on
In the menu you press → on "Flight". Here's the chain of calls:
```
Menu::ChangeValue(+1)
 └─ PowerManager::SetLevel(flight, player, 1)
     └─ SetActive(flight, player, true)
         ├─ exclusive group check (Flight has none, so nothing is turned off)
         ├─ flight.m_active = true; m_level = 1
         ├─ flight.OnActivate(player)
         └─ flight.OnLevelChanged(player, 1)
```
`Flight::OnActivate` prepares the poses and resets state:
```cpp
if (!IsPedFile(m_flyAnim.file))
    Command<Commands::REQUEST_ANIMATION>(m_flyAnim.file.c_str());   // "PARACHUTE"
m_diving = false;
m_velocity = CVector(0.0f, 0.0f, 0.0f);
```
Animations live in `anim\anim.img`, in files like `parachute.ifp`. Only the basic `PED` file is always in memory. `REQUEST_ANIMATION` asks the **streaming system** to load the parachute file from disk in the background. A few frames later `HAS_ANIMATION_LOADED("PARACHUTE")` becomes true.

### 9.2 Every frame: `Flight::Update(player, dt)`
`PowerManager::Update` calls it once per frame, from `gameProcessEvent`, while Flight is active. `dt` = `CTimer::ms_fTimeStep / 50` = seconds of *game* time this frame.

#### Step 1: take off (first frame only)
```cpp
if (!m_flying) {
    player->bApplyGravity = false;      // CPhysical flag: physics stops adding gravity
    player->bCollisionProof = true;     // no damage from hitting walls or the ground
    CVector pos = player->GetPosition();
    if (player->bIsStanding)
        player->SetPosn(pos.x, pos.y, pos.z + 0.5f);   // lift him off the ground
    m_flying = true;
}
player->bApplyGravity = false;          // re-applied EVERY frame
player->bCollisionProof = true;
```
- **Why lift 0.5 m?** A ped standing on the ground moves by its *walk animation*, not by physics. Lifting him makes the game treat him as airborne, and airborne peds are moved by `m_vecMoveSpeed`.
- **Why re-apply every frame?** The game resets some flags on its own: after cutscenes, deaths, entering interiors... Writing them every frame is cheap and keeps them reliable.

#### Step 2: read the controls
```cpp
CPad* pad = CPad::GetPad(0);
float forward = -pad->GetPedWalkUpDown() / 128.0f;      // W = +1, S = -1
float strafe  =  pad->GetPedWalkLeftRight() / 128.0f;   // D = +1, A = -1
float vertical = (Input::IsDown(m_ascendKey) ? 1.0f : 0.0f)
               - (Input::IsDown(m_descendKey) ? 1.0f : 0.0f);   // Space / C
```
- `GetPedWalkUpDown()` returns -128...128, the **same value the game uses to make CJ walk**. We get the player's keyboard or gamepad bindings for free, and when controls are disabled (menu open, cutscene) it returns 0, so CJ just hovers.
- The axis is inverted, because "up on the stick" is negative in the game's convention.
- Space and C aren't walk axes, so we read them with our own `Input`.

#### Step 3: where is "forward"? Ask the camera.
```cpp
CVector front = TheCamera.m_aCams[TheCamera.m_nActiveCam].m_vecFront;   // unit vector the camera looks along
if (!m_followCameraPitch) front.z = 0.0f;
front.Normalize();
CVector flatFront(front.x, front.y, 0.0f);
flatFront.Normalize();
CVector right(flatFront.y, -flatFront.x, 0.0f);   // flatFront rotated 90° clockwise
```
`TheCamera` is the game's global `CCamera` object. It holds up to 3 camera modes, and `m_nActiveCam` says which one is in use. `m_vecFront` is where it points. Because we use the camera's direction and not CJ's, **W always means "away from the screen"**, and looking up while pressing W makes him climb, like Superman.

The right vector comes from 2D rotation: rotating `(x, y)` by -90° gives `(y, -x)`.

#### Step 4: the target velocity
```cpp
int lvl = Level() - 1;                                       // 0, 1, 2
CVector target = (front * forward + right * strafe) * m_speed[lvl];   // m/s: 12, 30 or 60
target.z += vertical * m_climb[lvl];                         // m/s: 8, 18 or 35
```
Example: W held, camera pointing north and slightly up, level 2 → `target ≈ (0, 29.4, 6.0)` m/s.

#### Step 5: smooth it (easing)
Jumping straight to the target velocity would feel robotic. Instead, every frame we move a fraction of the way there:
```cpp
float timeScale = CTimer::ms_fTimeScale > 0.05f ? CTimer::ms_fTimeScale : 0.05f;
float realDt = dt / timeScale;                         // real seconds, even in Bullet Time
float blend = 1.0f - std::exp(-m_responsiveness * realDt);
m_velocity += (target - m_velocity) * blend;
```
This is **exponential smoothing**. With `Responsiveness = 4`, about 63% of the remaining difference is covered every 0.25 s, whatever the frame rate. Using `exp()` instead of a fixed fraction makes it behave the same at 30, 60 or 144 FPS. Releasing all keys makes the target 0, so CJ glides to a stop and hovers.

#### Step 6: hand it to the game's physics
```cpp
player->m_vecMoveSpeed = m_velocity / (kStepsPerSecond * timeScale);
```
- **Unit conversion:** `m_velocity` is in m/s, and the game wants **meters per step** (1/50 s), so we divide by 50.
- **Bullet Time:** under Bullet Time, steps get shorter (`timeStep × timeScale`), which would make CJ fly slowly too. Dividing by `timeScale` cancels that out, so Flight stays real-time.
- This single assignment writes 12 bytes at offset `0x44` of CJ's object. In the **next frame's** `CWorld::Process`, the game moves CJ by `m_vecMoveSpeed × timeStep`, checking his collision model against buildings, cars and objects. If he hits a wall, the game stops him. We never wrote collision code.

#### Step 7: face the direction of travel
```cpp
if (m_velocity.Magnitude2D() > 1.0f) {               // only when actually moving
    float heading = std::atan2(-m_velocity.x, m_velocity.y);
    player->m_fHeadingCurrent = heading;  // where he faces now
    player->m_fHeadingGoal = heading;     // where his AI wants to turn (stops it turning back)
    player->SetHeading(heading);          // rotate his matrix (what gets rendered)
}
```
`atan2(-x, y)` converts a direction into the game's heading convention: 0 = north, counter-clockwise positive. All three fields are set; otherwise the ped's own turning logic would rotate him back.

#### Step 8: the Superman pose
```cpp
float speed = m_velocity.Magnitude();
if (m_diving && speed < 2.0f)       m_diving = false;   // slow → hover pose
else if (!m_diving && speed > 4.0f) m_diving = true;    // fast → dive pose
const Anim& anim = m_diving ? m_flyAnim : m_hoverAnim;
```
The **gap between 2 and 4 m/s** (*hysteresis*) prevents flickering between poses when the speed hovers around one value. Then:
```cpp
if (!IsPedFile(anim.file) && !Command<Commands::HAS_ANIMATION_LOADED>(anim.file.c_str()))
    return;                                       // parachute anims still streaming in

int handle = CPools::GetPedRef(player);           // C++ pointer → script handle
if (!Command<Commands::IS_CHAR_PLAYING_ANIM>(handle, anim.name.c_str()))
    Command<Commands::TASK_PLAY_ANIM_NON_INTERRUPTABLE>(handle, anim.name.c_str(),
        anim.file.c_str(), 4.0f, true, false, false, false, -1);
```
- **Handles:** script commands identify peds by *handle* (an index into the ped pool plus a check value), not by pointer. `CPools::GetPedRef` converts.
- **The task system:** peds are driven by *tasks*: "walk to", "enter car", "play anim"... `TASK_PLAY_ANIM_NON_INTERRUPTABLE` gives CJ a task that loops the animation (`FALL_SkyDive_Accel` from `parachute.ifp`, or `Swim_Tread` from `ped.ifp`).
  - `4.0` = blend speed, so a new pose fades in over about a quarter second.
  - `loop = true`.
  - `lockX/lockY = false`, so the animation doesn't move him; our velocity does.
- **Self-healing:** the game sometimes interrupts tasks, for example with its "falling" reaction when a ped is airborne. We check `IS_CHAR_PLAYING_ANIM` every frame and restart the pose if it's gone.
- **Rendering:** the animation system blends the pose into CJ's skeleton, and RenderWare draws his skinned model with that skeleton in step 2 of the frame.

### 9.3 Frame-by-frame timeline
```
frame N    gameProcessEvent: Flight::Update
             reads W (pad), camera front → target (0, 29.4, 6.0) m/s
             eases m_velocity toward it → writes m_vecMoveSpeed = velocity / 50
             heading + pose task
           render: CJ drawn where he was, in the dive pose
frame N+1  CGame::Process → CWorld::Process
             gravity skipped (bApplyGravity = false)
             position += m_vecMoveSpeed × timeStep, collisions resolved
             animation advanced (dive loop)
           CCamera::Process: camera follows CJ to his new position
           gameProcessEvent: Flight::Update again (new camera direction, keys...)
           render: CJ drawn at the new position
...
```

### 9.4 Turning it off and landing safely
When you set Flight to Off (or CJ dies):
```cpp
void Flight::OnDeactivate(CPlayerPed* player) {
    StopFlying(player);                          // gravity on, clear pose task, unload PARACHUTE
    if (player->m_vecMoveSpeed.z > 0.0f)
        player->m_vecMoveSpeed.z = 0.0f;         // don't launch him upward
}
```
`StopFlying` turns gravity back on but **keeps** `bCollisionProof` and sets `m_landingGuard = true`. CJ now falls, possibly from 200 m. But `PowerManager` calls `UpdateAlways()` on every power **every frame, even inactive ones**:
```cpp
void Flight::UpdateAlways(CPlayerPed* player, float) {
    if (!m_landingGuard || IsActive()) return;
    if (player->bIsStanding || player->bInVehicle || player->bTouchingWater) {
        player->bCollisionProof = m_wasCollisionProof;   // restore what he had before
        m_landingGuard = false;
    }
}
```
So he lands without damage, and only then does his normal vulnerability come back. Restoring the *saved* value, not just `false`, matters: if Invulnerability was on, we don't break it.

### 9.5 Why this approach (and what we didn't do)
| Option | Problem |
|---|---|
| `SetPosn()` CJ every frame (teleporting) | no collisions: he'd fly through buildings |
| Keep gravity and push up harder | jittery, depends on frame rate |
| Write our own collision code | lots of work, and the game already has it |
| **Gravity off + set `m_vecMoveSpeed`** | the engine does movement, collisions and interpolation; we only decide the direction |

The general lesson: **find the engine's own mechanism and feed it inputs**, rather than fighting it.

---

## 10. The same idea in CLEO

For comparison, a minimal CLEO version of the core of Flight (not part of the repo):
```
{$CLEO .cs}
script_name 'fly'
while true
    wait 0
    if is_key_pressed KeyCode.F2
    then
        // CLEO can't flip bApplyGravity directly; we'd counter gravity with upward velocity
        float vx, vy, vz
        vx, vy, vz = get_char_velocity $scplayer
        set_char_velocity $scplayer {x} vx {y} vy {z} 0.5
    end
end
```
CLEO works through **opcodes**, the script VM's limited API. That's simple and safe, but you only get what Rockstar exposed. Native plugins work on **the engine objects themselves**: any field, any function. That's why the powers live in C++. The trade-off is that a mistake in native code crashes the whole game, while a CLEO mistake usually only stops the script.

---

## 11. Debugging, crashes and safety

- **`ArmageddonPlus.log`** is flushed after every line, so after a crash the last line shows what was happening.
- **A crash means a bad memory access.** Usual causes:
  - a **null pointer**. Always `if (!player) return;`.
  - a **dangling pointer**: the game deleted a car you kept a pointer to. Telekinesis re-checks `CPools::ms_pVehiclePool->IsObjectValid(m_held)` every frame.
  - a **wrong field**: plugin-sdk headers are reverse-engineered, and an example may use an outdated name. Trust the headers in `game_sa\`.
- **Leave the game as you found it:** save every flag you change and restore it on deactivate (`m_wasFireProof`, saved weapons, time scale...).
- **Version lock:** the `.asi` only works on 1.0 US. On another exe, the hooks would patch random bytes.
- **Your Steam install is never touched.** Everything happens in `C:\Games\GTA-SA-Modding`, and `GTA-SA-Modding-clean.zip` is the restore point.

---

## 12. Glossary
| Term | Meaning |
|---|---|
| **ASI** | A DLL renamed `.asi`, loaded into the game by an ASI loader |
| **ASI loader** | A fake `vorbisFile.dll` that loads every `.asi` at startup |
| **CLEO** | Plugin that runs custom scripts (`.cs`) in the game's script VM |
| **Opcode** | One script VM instruction, e.g. `00A5` create car |
| **Handle** | Script-side ID of a ped/vehicle/object (C++ uses pointers) |
| **Hook** | Redirecting a call or jump in the game's code to our code |
| **plugin-sdk** | Reverse-engineered headers and wrappers for the game's classes, functions and variables |
| **Pool** | The game's fixed-size arrays of peds, vehicles and objects (`CPools`) |
| **RenderWare** | The 3D engine underneath GTA SA |
| **Step** | The game's time unit: 1/50 s. Velocities are in meters per step |
| **Streaming** | Loading models and animations from `.img` archives on demand |
| **Task** | A unit of ped behavior (walk, enter car, play anim...) |
| **Time scale** | `CTimer::ms_fTimeScale`, the global game speed |
| **v1.0 US** | The original game exe that all addresses are known for |
