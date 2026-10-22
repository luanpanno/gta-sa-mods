# CLEO scripts

Sources are `.txt` files written in Sanny Builder 4 syntax. Compiling (F6) produces a `.cs` file. `scripts/deploy.ps1` copies every `.cs` in this folder to `C:\Games\GTA-SA-Modding\modloader\MyCleoMods\`. Modloader injects it into CLEO, so the game's own `cleo\` folder stays clean.

| Script | Phase | What it does |
|---|---|---|
| `hello.txt` | 1.1 | F5 shows a help box with a press counter |

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
5. If the compile shows errors, copy the message to me. It includes the line number.

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
- In Sanny, press **Ctrl+Space** to autocomplete commands and **F1** on a command to open its docs.
