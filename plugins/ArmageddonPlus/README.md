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
- `src/main.cpp`: entry point. A global object whose constructor runs at load time and subscribes to game events.
- `src/core/` *(Phase 2.3)*: PowerManager, Energy, Input, Menu, Config
- `src/powers/` *(Phase 3)*: one file per power

## Concepts (vs CLEO)
| CLEO | plugin-sdk |
|---|---|
| `while true / wait 0` loop | `Events::gameProcessEvent += fn`: the game calls you every frame |
| `display_text_formatted` each frame | `Events::drawHudEvent += fn` with `CFont::PrintString` |
| `$scplayer` (a handle) | `FindPlayerPed()` returns a `CPlayerPed*`: the real engine object |
| `get_char_coordinates` | `ped->GetPosition()`. Fields like `m_fHealth` and `m_vecMoveSpeed` are readable/writable directly |
| opcodes | Engine classes in `C:\dev\plugin-sdk\plugin_sa\game_sa\*.h` (CPed, CVehicle, CWorld, CExplosion, CTimer...) |

**Careful:** you're inside the game process. A null pointer or a bad write crashes the game instantly, with no error message. Check pointers (`if (!player) return;`) and watch the log.

Screen coordinates: `SCREEN_COORD(x)` scales a 640x448-style value to the real resolution. `SCREEN_COORD_RIGHT(x)` measures from the right edge.
