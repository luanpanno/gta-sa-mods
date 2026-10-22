# ArmageddonPlus (C++ / plugin-sdk)

An `.asi` plugin: a Windows DLL with a different extension. The ASI loader (bundled with CLEO) loads it into `gta_sa.exe` at startup. From then on our code runs inside the game process and can call engine functions and read and write engine objects directly, through plugin-sdk's headers.

## Build and run
```powershell
.\scripts\run.ps1                 # build Release + deploy + start the game
.\scripts\build.ps1               # build only
.\scripts\build.ps1 -Configuration Debug
```
In VSCode: **Ctrl+Shift+B** builds. *Terminal -> Run Task -> "Build + deploy + run game"* does everything.
Close the game before building: Windows locks a loaded `.asi`.

Output: `bin\Release\ArmageddonPlus.asi` is deployed to `C:\Games\GTA-SA-Modding\modloader\ArmageddonPlus\`.
Log: `C:\Games\GTA-SA-Modding\ArmageddonPlus.log`.

## Layout
- `src/main.cpp`: entry point. A global object whose constructor runs at load time. It registers the powers and subscribes to game events.
- `src/core/`
  - `Log`: `ArmageddonPlus.log` in the game folder, flushed per line so it survives crashes
  - `Config`: reads `ArmageddonPlus.ini` (repo: `config/ArmageddonPlus.ini`) and parses key names like `F10`
  - `Input`: keyboard sampled once per frame (`IsDown`, `JustPressed`, `JustReleased`)
  - `Power`: base class for every power (Toggle / Hold / OneShot)
  - `PowerManager`: on/off and levels, optional hotkeys (`Key=` in the ini), per-frame `Update(dt)`, switches everything off on death, top-right power list
  - `Menu`: **F10** opens the power menu, the main way to use powers. Up/Down selects, Left/Right sets the level (Off/1/2/3), Enter toggles a power or fires a one-shot one, Backspace or F10 closes. While it's open, CJ's controls are disabled with the game's own script command `Command<Commands::SET_PLAYER_CONTROL>` (same as CLEO's `set_player_control`), and any hotkeys are paused. Drawn with `CSprite2d::DrawRect` + `CFont`. New powers show up in it automatically. **Never use F1** for anything: in this game F1 makes it stop reading the keyboard (the mouse still works) until restart. That was the "CJ frozen after the menu" bug.
- `src/powers/`: one class per power
  - `SuperSpeed` (3 levels): after the game moves CJ each frame, push him further the same way (`MultiplierN`), with line-of-sight checks so he never passes through walls. Leg animations are sped up and fatigue is off. **Super jump:** on take-off (was standing, now airborne and going up), his vertical speed is replaced with `JumpN`, and he's collision-proof until he lands, so there's no fall damage.
  - `ExplosionPunch` (3 levels): every frame it checks the peds and vehicles near CJ. If something's health dropped, CJ was the last attacker (`m_pLastEntityDamage`) and the weapon was melee, it calls `CExplosion::AddExplosion` at the victim (grenade / rocket / car bomb). CJ is explosion-proof while it's on.
  - `Invulnerability` (on/off): proof flags on CJ and his current vehicle
  - `SpawnVehicle` (one-shot): the CLEO spawner ported to engine classes (`CStreaming`, `CModelInfo`, `CAutomobile`...)

## Levels
A Toggle power that overrides `MaxLevel()` (e.g. `return 3;`) gets levels for free. In the menu, Left/Right picks the level; an optional hotkey steps off -> 1 -> 2 -> 3 -> off, the overlay shows `Super Speed 2/3`, and `OnLevelChanged(player, level)` / `Level()` tell the power which level it's on.

## Adding a power
1. Create `src/powers/MyPower.h/.cpp`, subclass `Power`, and implement `Name()`, `Id()` and the hooks you need (`OnActivate`, `Update`, `OnDeactivate`).
2. Register it in `main.cpp`: `g_powers.Register(std::make_unique<MyPower>());`
3. Optional: add a `[Power.MyPower]` section with tuning values (and an optional `Key=`) to `config/ArmageddonPlus.ini`, and read them in `LoadConfig`.

The `.vcxproj` compiles every `src/**/*.cpp` automatically.

## Concepts (vs CLEO)
| CLEO | plugin-sdk |
|---|---|
| `while true / wait 0` loop | `Events::gameProcessEvent += fn`: the game calls you every frame |
| `display_text_formatted` each frame | `Events::drawHudEvent += fn` with `CFont::PrintString` |
| `$scplayer` (a handle) | `FindPlayerPed()` returns a `CPlayerPed*`: the real engine object |
| `get_char_coordinates` | `ped->GetPosition()`. Fields like `m_fHealth` and `m_vecMoveSpeed` are readable/writable directly |
| opcodes | Engine classes in `C:\dev\plugin-sdk\plugin_sa\game_sa\*.h` (CPed, CVehicle, CWorld, CExplosion, CTimer...) |

**Header names win over examples.** Some plugin-sdk examples are out of date: `m_fCurrentRotation` is now `m_fHeadingCurrent`, proof flags are plain `bBulletProof` members of `CPhysical`, and so on. When something doesn't compile, search `C:\dev\plugin-sdk\plugin_sa\game_sa\*.h`.

**Careful:** you're inside the game process. A null pointer or a bad write crashes the game instantly, with no error message. Check pointers (`if (!player) return;`) and watch the log.

Screen coordinates: `SCREEN_COORD(x)` scales a 640x448-style value to the real resolution. `SCREEN_COORD_RIGHT(x)` measures from the right edge.
