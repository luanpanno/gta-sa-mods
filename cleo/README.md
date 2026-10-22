# CLEO scripts

The learning projects from Phase 1. Sources are `.txt` files in Sanny Builder 4 syntax. Compiling (F6) writes a `.cs` next to each one (git-ignored). `scripts/deploy.ps1` copies every `.cs` to `C:\Games\GTA-SA-Modding\modloader\MyCleoMods\`, and Modloader injects them into CLEO, so the game's own `cleo\` folder stays clean.

| Script | Keys | What it does |
|---|---|---|
| [hello.txt](hello.txt) | F5 | Help box with a press counter |
| [vehicle_spawner.txt](vehicle_spawner.txt) | F7/F8 choose, F6 spawn, Shift+F6 spawn + enter | 12 vehicles: Infernus, Turismo, Cheetah, Banshee, NRG-500, Sanchez, BMX, Monster, Rhino, Maverick, Hunter, Hydra |
| [console.txt](console.txt) | F9 open/close, Enter run, Backspace delete | Command console, see below |

### Console commands
| Command | Example |
|---|---|
| `car <name\|id>` | `car infernus`, `car 411`, `car hydra` (any of the 212 vehicles, by game name) |
| `weapon <name>` | `weapon minigun`. Also pistol, deagle, shotgun, spas, uzi, mp5, ak47, m4, sniper, rpg, flamethrower, grenade, katana, chainsaw, parachute |
| `money <amount>` | `money 1000000` |
| `wanted <0-6>` | `wanted 0` |
| `health` | Full health + armor |
| `fix` | Repair the current vehicle |
| `time <0-23>` | `time 12` |
| `weather <id\|auto>` | `weather 8` (storm), `weather 9` (fog), `weather auto` |
| `tp <place>` | `tp marker` (map waypoint), `grove`, `sf`, `lv`, `chiliad`, or a saved name |
| `save <name>` | Save CJ's position (works in interiors) to `C:\Games\GTA-SA-Modding\gta-sa-mods.ini` |
| `god` / `ammo` / `cops` / `stamina` | Toggles, shown in the top-left while on |

## Sanny Builder workflow
**First run:** Game = GTA San Andreas, Edit mode = `GTA SA (v1.0 - SBL)` (also in the bottom-right corner of the status bar), Game directory = `C:\Games\GTA-SA-Modding`.

1. *File → Open* `C:\dev\gta-sa-mods\cleo\<script>.txt`.
2. Press **F6** (*Run → Compile*). The `.cs` appears next to the `.txt`.
3. From `C:\dev\gta-sa-mods`, run `.\scripts\run.ps1 -NoBuild` to deploy and start the game.

- **Edited a file outside Sanny** (VSCode, git pull...)? Close its tab in Sanny **without saving** and reopen it. Otherwise Sanny saves its old copy over the file when compiling.
- **Save as UTF-8 without BOM.** A BOM gives *"Unknown directive ï»¿"*.
- If a `.cs` also ends up in `C:\Games\GTA-SA-Modding\cleo\`, delete it there, or the script runs twice.
- Not loading? `cleo.log` (game folder) lists started scripts. `modloader\modloader.log` shows whether Modloader found `MyCleoMods\*.cs`.
- In Sanny, **Ctrl+Space** autocompletes and **F1** on a command opens its docs.

More fixes: [docs/troubleshooting.md](../docs/troubleshooting.md#cleo--sanny-builder).

## Cheat sheet

### Basics (hello.txt)
- `{$CLEO .cs}` compiles a standalone CLEO script instead of the game's main script.
- `{text}`, `{time}` and other braces are optional inline labels for arguments, for readability only.
- `while true ... wait 0 ... end` is the game loop. Every iteration **must** call `wait`: scripts share one thread with the game, so a loop without `wait` freezes it.
- `$player1` is the player. `$scplayer` is CJ's character (ped).
- `~g~ ~y~ ~r~ ~s~` are text colors (green, yellow, red, reset). `~n~` is a new line.
- `KeyCode.F5` is a key name. The full list is in `C:\Games\GTA-SA-Modding\cleo\.config\enums.json`.

### Functions, models, handles (vehicle_spawner.txt)
- `function NAME(arg: int): int ... return x ... end`: a function with its own local variables.
  - If it returns a value, the call **must** store it (`x = NAME(1)`), even if you don't need it.
  - Don't pass enum values like `KeyCode.F6` into your own functions. That gives error 0049. Pass them to built-in commands only, or store them in an `int` first.
- Variable names can't match a class (`car`, `char`, `player`, `weapon`, `object`, `blip`...) or a command (`line`...), in any letter case.
- `const VEHICLE_COUNT = 12` is a compile-time constant. `switch / case / default` and `continue` work like in JS.
- `#INFERNUS` is a model name, compiled to ID 411 (from `data\vehicles.ide`).
- **Model streaming:** `request_model` → wait for `has_model_loaded` → create → `mark_model_as_no_longer_needed`.
- **Handles:** `create_car` returns an int handle. `mark_car_as_no_longer_needed` hands the car back to the game, which deletes it once you're far away.
- `get_offset_from_char_in_world_coords $scplayer 0.0 6.0 0.5` gives the point 6 m in front of CJ (x = right, y = forward, z = up, relative to him).

### Text input, strings, memory (console.txt)
- **Strings as memory buffers:** `allocate_memory 64` returns a pointer (an int), and CLEO 5 accepts it anywhere a string is expected (`%s`, `is_text_equal`...). It's C's `char*`.
- `read_memory` / `write_memory` work on single bytes. We write ASCII codes plus a `0` terminator.
- `get_key_just_pressed_in_range A Z` returns the key pressed this frame. Letter key codes equal uppercase ASCII; +32 makes them lowercase.
- `2080: is_key_just_pressed`: an **opcode prefix** picks one specific command. Two plugins define that name, and we want CLEO 5's (2080), not CLEO+'s (0E3D, not installed).
- `if x, y, z = get_target_blip_coords then` is a command that returns values *and* acts as a condition.
- Teleports store only X/Y. After `load_scene`, `get_ground_z_for_3d_coord` finds the height.

### State and persistence (console.txt, QoL)
- **Functions get copies of their arguments**, so they can't change the caller's variables. `RUN_COMMAND` *returns* a code (`TOGGLE_GOD`...), and the main loop flips its own flag.
- **Toggles are re-applied every frame:** the game resets proofs and the like on death or vehicle changes.
- **INI files** (`write_float_to_ini_file` / `read_float_from_ini_file`) give simple persistence. Path prefixes: `root:\` = game folder, `user:\` = save folder, `.\` = the script's folder.
- **Interiors** are "areas": 0 is outside, and each interior has its own number. Teleporting into one needs `set_area_visible` + `set_char_area_visible`, or CJ floats in a void.
