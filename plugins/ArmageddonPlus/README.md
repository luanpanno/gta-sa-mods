# ArmageddonPlus (C++ / plugin-sdk)

The superpowers plugin. How to *use* the powers: [docs/powers.md](../../docs/powers.md). This page is about how the plugin works and how to extend it. For the full story of how code ends up running inside the game, read [docs/how-a-mod-works.md](../../docs/how-a-mod-works.md).

An `.asi` is a Windows DLL with a different extension. The ASI loader (bundled with CLEO) loads it into `gta_sa.exe` at startup. From then on, our code runs **inside the game process**: it can call engine functions and read and write engine objects directly, through [plugin-sdk](https://github.com/DK22Pac/plugin-sdk)'s headers.

## Build and run
```powershell
.\scripts\run.ps1                          # build Release + deploy + start the game
.\scripts\build.ps1                        # build only
.\scripts\build.ps1 -Configuration Debug   # with symbols (links Plugin_d.lib)
```
- VSCode: **Ctrl+Shift+B** builds. *Terminal → Run Task → "Build + deploy + run game"* does everything.
- Output: `bin\Release\ArmageddonPlus.asi` is deployed to `C:\Games\GTA-SA-Modding\modloader\ArmageddonPlus\`, together with `config\ArmageddonPlus.ini`.
- Log: `C:\Games\GTA-SA-Modding\ArmageddonPlus.log`.
- Close the game before building: Windows locks a loaded `.asi`.

The project ([ArmageddonPlus.vcxproj](ArmageddonPlus.vcxproj)) mirrors the settings of plugin-sdk's generated examples: Win32, v143, C++latest, static runtime, `PLUGIN_SGV_10US`. It compiles every `src/**/*.cpp`, so new files need no project edits.

## Architecture
```
main.cpp        global object whose constructor runs at load time ("main"):
                registers the powers, loads the ini, hooks Events::gameProcessEvent / drawHudEvent
core/
  Log           ArmageddonPlus.log, flushed per line (survives crashes)
  Config        reads the ini; key names ("F10", "NUMPAD5", "SHIFT") -> virtual-key codes
  Input         keyboard/mouse sampled once per frame: IsDown / JustPressed / JustReleased
  Power         base class of every power
  PowerManager  owns the powers: on/off, levels, exclusive groups, optional hotkeys,
                per-frame updates, switch-off on death, top-right power list
  Menu          F10 power menu (arrows / Enter / Backspace)
powers/         one class per power
```

Every game frame (`gameProcessEvent`, after the game has processed the world):
1. `Input::Update()`
2. `Menu::Update(dt)`, before the powers, because it pauses the hotkeys while open
3. `PowerManager::Update()`: hotkeys, then `Update()` on active powers and `UpdateAlways()` on all of them

Every frame while the HUD draws (`drawHudEvent`): the powers' `Draw()` (screen effects), the watermark, the power list (hidden while the menu is open), then the menu.

## The `Power` class
| Override | Purpose |
|---|---|
| `Name()`, `Id()` | Menu label; ini section `[Power.<Id>]` |
| `GetMode()` | `Toggle` (default), `Hold` (active while a key is held; needs `Key=`), `OneShot` (fires once) |
| `MaxLevel()` | > 1 gives levels: the menu's ←/→ pick Off/1/2/3; `Level()` returns the current one |
| `ExclusiveGroup()` | Powers with the same group string can't be on together (`"mouse"`: Telekinesis, Force, Lightning Strike) |
| `LoadConfig(config, section)` | Read tuning values once at startup |
| `OnActivate` / `OnDeactivate` | Turned on / off (also off when CJ dies). OneShot: `OnActivate` = fire |
| `OnLevelChanged(player, level)` | After activation and on every level change |
| `Update(player, dt)` | Every frame while on. `dt` = game seconds (slowed by Bullet Time) |
| `UpdateAlways(player, dt)` | Every frame, on or off. For clean-up after switching off, e.g. Flight's landing protection |
| `Draw()` | HUD/screen drawing while on, e.g. Bullet Time's tint, Lightning's bolt and crosshair |

### Adding a power
1. Create `src/powers/MyPower.h/.cpp`, subclass `Power`, and implement `Name()`, `Id()` and the hooks you need.
2. Register it in `main.cpp`: `g_powers.Register(std::make_unique<MyPower>());`. Registration order = menu order.
3. Add a `[Power.MyPower]` section with tuning values to `config/ArmageddonPlus.ini` and read them in `LoadConfig`.

It shows up in the menu automatically.

### Rules of thumb (learned the hard way)
- **Re-apply state every frame** (proof flags, gravity, time scale...). The game resets things after deaths, cutscenes and menus.
- **Save and restore** whatever you change (`m_wasFireProof`, saved weapons...), so turning a power off leaves CJ as he was.
- **Never keep a raw pointer to a game entity without re-checking it.** The game deletes peds, cars and objects at any time. Check with `CPools::ms_pVehiclePool->IsObjectValid(ptr)` (and the ped/object pools) every frame.
- **Physics units:** `m_vecMoveSpeed` is meters per *time step* (1/50 s), so `velocity_m_s / 50`. Under Bullet Time, steps shrink by `CTimer::ms_fTimeScale`.
- **Parked cars and placed objects are asleep** (`bIsStatic`). Call `SetIsStatic(false)` before pushing them.
- **Script commands are available from C++:** `Command<Commands::SET_PLAYER_CONTROL>(0, false)` runs the same code as the CLEO opcode. That's safer than poking flags whose meaning you don't fully know.
- **Never use F1** as a key: it makes GTA SA stop reading the keyboard.

## How each power works
| Power | Technique |
|---|---|
| `Flight` | Gravity off (`bApplyGravity`). Every frame, `m_vecMoveSpeed` eases toward the key direction (camera-relative), and the game's physics moves CJ **with collisions**. Collision-proof until he lands (`UpdateAlways`). Poses: `FALL_SkyDive_Accel` (PARACHUTE) while moving and `Swim_Tread` (PED) while hovering, via `TASK_PLAY_ANIM_NON_INTERRUPTABLE`, restarted if interrupted. |
| `SuperSpeed` | After the game moves CJ, push him further the same way (`MultiplierN`), with two line-of-sight checks (waist and knee) so he never enters walls. Leg anims sped up, fatigue off. Super jump: on take-off (was standing, now airborne going up), the vertical speed is replaced with `JumpN`, collision-proof until landing. |
| `ExplosionPunch` | Tracks the health of nearby peds and vehicles. A drop + `m_pLastEntityDamage == CJ` + melee weapon = a punch → `CExplosion::AddExplosion` at the victim. |
| `Telekinesis` | Gives a non-firing Desert Eagle for the crosshair (the previous slot weapon is saved and restored). The ray comes from `CCamera::Find3rdPersonCamTargetVector` + `CWorld::ProcessLineOfSight` (`pIgnoreEntity` = CJ). The held entity gets gravity off and a velocity toward a point along the aim. Releasing throws it. |
| `Force` | Everything in a ~140° cone within the radius gets a velocity away from or toward CJ, plus lift. Peds are lifted 0.3 m first so physics moves them, not their walk animation. |
| `LightningStrike` | Camera ray → explosion at the hit point. The bolt: jagged 3D points projected with `CSprite::CalcScreenCoors`, drawn as quads (`CSprite2d::Draw2DPolygon`), glow + core, plus a screen flash. Timed with `GetTickCount64` so Bullet Time doesn't slow it. |
| `FireTrail` | `gFireManager.StartFire` every `Spacing` meters at CJ's feet, or behind his car. CJ and the car are fireproof. |
| `BulletTime` | `CTimer::ms_fTimeScale` slows the game. Every animation on CJ runs `1/scale` faster, and since peds move by their animations, he stays real-time. Each anim's own speed is remembered, so it stacks with Super Speed. |
| `Invulnerability` | All proof flags on CJ and his current vehicle, and health kept full. |
| `SpawnVehicle` | `CStreaming::RequestModel` → the right vehicle class for the model type → `Teleport` (adds it to the world). Adapted from plugin-sdk's `CreateCar` example. |

## plugin-sdk vs CLEO
| CLEO | plugin-sdk |
|---|---|
| `while true / wait 0` loop | `Events::gameProcessEvent += fn`: the game calls you every frame |
| `display_text_formatted` each frame | `Events::drawHudEvent += fn` with `CFont::PrintString` |
| `$scplayer` (a handle) | `FindPlayerPed()` returns a `CPlayerPed*`: the real engine object |
| `get_char_coordinates` | `ped->GetPosition()`. Fields like `m_fHealth` and `m_vecMoveSpeed` are readable/writable directly |
| opcodes | Engine classes in `C:\dev\plugin-sdk\plugin_sa\game_sa\*.h` (CPed, CVehicle, CWorld, CExplosion, CTimer...), or `Command<Commands::...>` |

- **Headers win over examples.** Some plugin-sdk examples are out of date: `m_fCurrentRotation` is now `m_fHeadingCurrent`, proof flags are plain `bBulletProof` members of `CPhysical`, and so on. When something doesn't compile, search `C:\dev\plugin-sdk\plugin_sa\game_sa\*.h`.
- **You're inside the game process.** A null pointer or a bad write crashes the game instantly, with no error message. Check pointers and watch the log.
- **Screen coordinates:** `SCREEN_COORD(x)` scales a value designed for a **1080-pixel-tall** screen to the real resolution (`x × shorter side / 1080`). `SCREEN_COORD_RIGHT(x)` measures from the right edge. Details: [how-a-mod-works.md §8.5](../../docs/how-a-mod-works.md#85-screen-coordinates).
