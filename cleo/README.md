# CLEO scripts

Sources are `.txt` files written in Sanny Builder 4 syntax. Compiling (F6) produces a `.cs` file. `scripts/deploy.ps1` copies every `.cs` in this folder to `C:\Games\GTA-SA-Modding\modloader\MyCleoMods\`. Modloader injects it into CLEO, so the game's own `cleo\` folder stays clean.

| Script | Phase | What it does |
|---|---|---|
| `hello.txt` | 1.1 | F5 shows a help box with a press counter |
| `vehicle_spawner.txt` | 1.2 | F7/F8 chooses one of 12 vehicles, F6 spawns it in front of CJ, Shift+F6 spawns it and puts CJ in it |
| `console.txt` | 1.3 | F9 opens a command console: `car infernus`, `car 411`, `weapon minigun`, `money 1000000`, `wanted 0`, `health`, `tp marker` / `grove` / `sf` / `lv` / `chiliad` |

## Sanny Builder: first-time setup
1. Run `C:\Tools\SannyBuilder\sanny.exe`.
2. If the setup wizard asks:
   - **Game:** GTA San Andreas
   - **Edit mode:** `GTA SA (v1.0 - SBL)`. The mode picker is also in the bottom-right corner of the status bar.
   - **Game directory:** `C:\Games\GTA-SA-Modding`
3. Optional: *Tools -> Options -> General*. Turn off anything that auto-copies compiled scripts into the game folder. We deploy through Modloader instead.

## Compile a script
1. *File -> Open* and pick `C:\dev\gta-sa-mods\cleo\hello.txt`.
2. Check that the status bar mode is **GTA SA (v1.0 - SBL)**.
3. Press **F6** (*Run -> Compile*).
4. Check that `hello.cs` appeared **next to** `hello.txt`, in `C:\dev\gta-sa-mods\cleo\`.
   - If Sanny saved it into `C:\Games\GTA-SA-Modding\cleo\` instead, delete that copy and tell me. Otherwise the script loads twice (once from there, once from Modloader) and every message shows up doubled.
5. If the compile shows errors, copy the message to me.

> **When Claude edits a file you have open in Sanny:** close its tab **without saving** and reopen it before compiling. Otherwise Sanny saves its old copy over the new one when you compile.

## Run it
In PowerShell, from `C:\dev\gta-sa-mods`:
```powershell
.\scripts\run.ps1
```
This copies `hello.cs` to `modloader\MyCleoMods\` and starts the game. Load a save or start a new game. After about 3 s you should see *"gta-sa-mods loaded. Press F5."* Then press F5.

## If it doesn't load
- `C:\Games\GTA-SA-Modding\cleo.log` lists the scripts CLEO started. `hello.cs` should be in it.
- `C:\Games\GTA-SA-Modding\modloader\modloader.log` shows whether Modloader found `MyCleoMods\hello.cs`.

## Cheat sheet (concepts from hello.txt)
- `{$CLEO .cs}` makes Sanny compile a standalone CLEO script instead of the game's main script.
- `{text}`, `{time}` and other braces are just inline labels for the arguments. They're optional and only there to help you read the line.
- `while true ... wait 0 ... end` is the game loop. Every loop iteration **must** call `wait`. Scripts share one thread with the game, so a loop with no `wait` freezes the game.
- `$player1` is the player. `$scplayer` is CJ's character (ped).
- `~g~ ~y~ ~s~` are text colors (green, yellow, reset). `~n~` is a new line.
- `KeyCode.F5` is a key name. The full list is in `C:\Games\GTA-SA-Modding\cleo\.config\enums.json`.

### New in vehicle_spawner.txt
- `function NAME(arg: int): int ... return x ... end` defines a function with its own local variables. Call it like `x = NAME(1)`. If a function returns a value, Sanny **requires** you to store it in a variable, even when you don't need it. Don't pass enum values like `KeyCode.F6` straight into your own functions; that gives error 0049 "Not enough parameters". Use them directly in commands, or store them in an `int` variable first.
- Variable names can't match a class name, in any letter case. For example `car`, `char`, `player`, `object` and `blip` are all taken. Use `spawnedCar`, `myPed` and so on.
- Variable names also can't match a command name. For example `line` is a drawing command, so it's taken.

### New in console.txt
- **Strings as memory buffers:** `allocate_memory 64` returns a pointer (an int) to 64 bytes. CLEO 5 accepts that pointer anywhere a string is expected (`%s`, `is_text_equal`...). It's the C-style `char*`.
- `read_memory` / `write_memory` read and write single bytes. We write ASCII codes plus a `0` terminator to build the typed text.
- `get_key_just_pressed_in_range A Z` returns which key went down this frame. Letter key codes equal uppercase ASCII, and +32 makes them lowercase.
- `2080: is_key_just_pressed` uses an **opcode prefix** to pick one specific command. Two plugins define that name, and we want CLEO 5's (2080), not CLEO+'s (0E3D, which isn't installed).
- `if x, y, z = get_target_blip_coords then` is a command that both returns values and acts as a condition (false when no waypoint is set).
- `get_ground_z_for_3d_coord` after `load_scene` lets a teleport store only X/Y. The height comes from the ground.
- `const VEHICLE_COUNT = 12` is a compile-time constant.
- `#INFERNUS` is a model name, which Sanny turns into ID 411 (from `data\vehicles.ide`).
- **Model streaming:** the game only keeps a few models in memory. So you call `request_model`, wait until `has_model_loaded`, create the car, then `mark_model_as_no_longer_needed`.
- **Handles:** `create_car` returns a handle (an int) that refers to that car. `mark_car_as_no_longer_needed` hands it back to the game, which deletes it once you're far away.
- `get_offset_from_char_in_world_coords $scplayer 0.0 6.0 0.5` gives the point 6 m in front of CJ (x = right, y = forward, z = up, relative to CJ).
- `continue` skips to the next iteration of the loop. `switch/case/default` works like JS.

- In Sanny, press **Ctrl+Space** to autocomplete commands and **F1** on a command to open its docs.
