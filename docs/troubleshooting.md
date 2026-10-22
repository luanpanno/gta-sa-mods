# Troubleshooting

Problems we've actually run into, and how they were fixed.

## Where to look first
| Log | Written by | Location |
|---|---|---|
| `ArmageddonPlus.log` | our C++ plugin: powers loaded, keys, on/off events | `C:\Games\GTA-SA-Modding\` |
| `cleo.log` | CLEO: which `.cs` scripts started, script errors | `C:\Games\GTA-SA-Modding\` |
| `modloader.log` | Modloader: which mod files it found and loaded | `C:\Games\GTA-SA-Modding\modloader\` |

`ArmageddonPlus.log` is flushed after every line, so it's still complete after a crash. The last line shows what was happening.

---

## Game / controls

### CJ can't move, jump or enter cars, but the mouse camera still works
**Cause:** the **F1** key. In GTA SA, F1 makes the game stop reading the keyboard until it restarts.
**Fix:** restart the game, and never bind anything to F1. The plugin refuses F1 as the menu key and falls back to F10.

### CJ is frozen after closing the CLEO console (F9)
The console turns controls back on when it closes. If CJ is still frozen, open and close the console once more, or load a save.

### Powers don't respond to their old F-keys
Powers are controlled from the **F10 menu**. Hotkeys are optional: add `Key=...` to a power's section in `config/ArmageddonPlus.ini`.

### I can't shoot or punch
A **mouse power** is on: Telekinesis, Force or Lightning Strike. Turn it off in the menu.

### My pistol is now a Desert Eagle
Telekinesis puts a Desert Eagle in the pistol slot while it's on. Turning Telekinesis off gives your own pistol back, with its ammo. If CJ dies while it's on, the game takes all weapons anyway.

---

## Building / deploying

### `run.ps1` says "The game is running"
Windows locks a loaded `.asi`. Close the game, then run it again.

### `plugin-sdk not found/built`
`PLUGIN_SDK_DIR` isn't set, or the SDK isn't built. See [setup.md, step 6](setup.md#6-plugin-sdk). Restart VSCode and terminals after setting the variable.

### C++ compile error: "is not a member of ..." with a name taken from a plugin-sdk example
Some plugin-sdk examples are out of date. For example, `m_fCurrentRotation` is now `m_fHeadingCurrent`, and `ModelInfoType::MODEL_INFO_VEHICLE` is now `MODEL_INFO_VEHICLE`. Search the real names in `C:\dev\plugin-sdk\plugin_sa\game_sa\*.h`.

### `git commit` fails from PowerShell with "pathspec ... did not match"
Double quotes inside a `-m "..."` message break Windows PowerShell's argument passing. Write the message to a file and use `git commit -F <file>`.

---

## CLEO / Sanny Builder

### "Unknown directive ï»¿"
The `.txt` was saved as **UTF-8 with BOM**. Windows PowerShell 5's `Set-Content -Encoding utf8` adds one. Save the file without a BOM: VSCode's status bar → *UTF-8*, or Sanny's own editor.

### My edit disappeared after compiling
Sanny Builder still had the old version open and saved it over the file when compiling. After editing a script outside Sanny, close its tab **without saving** and reopen it.

### Error 0049 "Not enough parameters"
Usually an invalid argument, not a missing one. Example: passing `KeyCode.F6` straight into your own `function`. Pass enums only to built-in commands, or store them in an `int` variable first.

### "Expected 1 variables to store function's result"
A function that returns a value was called without storing the result. Write `x = MY_FUNC(...)`, even if you don't need `x`.

### "Invalid variable name ... reserved"
The variable name matches a Sanny class or command, in any letter case: `car`, `char`, `player`, `weapon`, `line`... Rename it, e.g. `spawnedCar`, `currentWeapon`, `inputText`.

### Script loads twice (doubled messages)
The `.cs` exists both in `C:\Games\GTA-SA-Modding\cleo\` and in `modloader\MyCleoMods\`. Delete the copy in `cleo\`; we deploy through Modloader only.

---

## Restoring the game
- **Mods broke something:** delete `C:\Games\GTA-SA-Modding` and extract `C:\Games\GTA-SA-Modding-clean.zip` in its place. That gives you a clean v1.0 + CLEO + SilentPatch + Modloader install. Then run `.\scripts\deploy.ps1`.
- **Steam install:** never modified. Steam → right-click the game → *Properties → Installed Files → Verify integrity of game files*.
