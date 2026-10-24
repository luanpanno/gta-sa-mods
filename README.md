# gta-sa-mods

Mods for **Grand Theft Auto: San Andreas** (PC), built step by step: from small CLEO scripts up to **ArmageddonPlus**, a C++ plugin that gives CJ superpowers. It's inspired by the old *GTA SA Armageddon* mod: flight, super speed, telekinesis, lightning, bullet time and more, all controlled from an in-game menu.

> **Single-player only, no ban risk.** GTA SA has no online mode and no anti-cheat. All modding happens in a separate copy of the game (`C:\Games\GTA-SA-Modding`); the Steam install is never touched, and Steam's "Verify integrity of game files" can always restore it.

## Contents
- [What's included](#whats-included)
- [Requirements](#requirements)
- [Getting started](#getting-started)
- [Day-to-day workflow](#day-to-day-workflow)
- [Project structure](#project-structure)
- [Configuration](#configuration)
- [Roadmap](#roadmap)
- [Documentation](#documentation)
- [Credits](#credits)

## What's included

### ArmageddonPlus: superpowers (C++ plugin)
Press **F10** in game to open the power menu: **↑/↓** select, **←/→** set the level, **Enter** toggles, **Insert** binds a shortcut key (e.g. X: Flight off → 1 → 2 → 3 → off), **Backspace** closes. Full controls and tips: [docs/powers.md](docs/powers.md).

| Power | Levels | What it does | How to use it once it's on |
|---|---|---|---|
| Flight | 3 | Superman-style flight with dive / hover poses | W/S toward/away from the camera, A/D strafe, Space/C up/down |
| Super Speed | 3 | Run 2.5x / 4.5x / 8x faster, walk on water, swim faster; running jumps keep the speed (add Super Jump for height) | Run and jump normally |
| Super Jump | 3 | Jumps up to ~30 / 83 / 130 m high and much longer, landing on his feet; level 3 lands with a ground-pound shockwave | Jump normally |
| Explosion Punch | 3 | Melee hits make the victim explode (cars too) | Punch / hit with a melee weapon |
| Telekinesis | 3 | Grab, carry and throw cars, people and objects | Aim (RMB) + click (LMB), hold to carry, release to throw |
| Force Push / Pull | 3 | Shockwave away from or toward CJ | LMB push, RMB pull |
| Lightning Strike | 3 | Call down a lightning bolt that blows up the target | Point the center crosshair, LMB |
| Fire Trail | 3 | Leaves a trail of fire on foot or behind his car | Just move |
| Bullet Time | 3 | World slows to 60% / 35% / 15%, CJ stays fast | Automatic while on |
| Invulnerability | on/off | CJ and his current car can't be damaged | Automatic while on |

The menu's **Spawn** section has pickers: **←/→** choose, **Enter** uses.

| Tool | What it does |
|---|---|
| Spawn Vehicle | Any of the game's ~200 spawnable vehicles, by name |
| Give Weapon | Any weapon, with ammo |
| Add Ammo | Refill the current weapon or all guns (up to max) |
| Add Money | $1,000 to $10,000,000 |

The **Cheats** row opens a page with all 91 built-in cheats, grouped and named. One-time cheats run with Enter, toggle cheats switch on/off, and active cheats are shown in the HUD.

The menu's **World** section has mods that change the world itself:

| World mod | Levels | What it does |
|---|---|---|
| Gravity | Moon / Mars / Zero-G / Heavy | Gravity for everything: CJ, people, cars, objects |
| Vehicle Handling | Sport / Race / Rocket | While driving: Shift turbo, J jump, extra grip, burst-proof tyres, auto-flip |
| Chaos | Meteors / Exploding Traffic / Flying Cars / Riot | Random mayhem around CJ (mission cars are spared) |
| NPC Reactions | Fear / Fear + Police | People flee when CJ uses powers, and optionally the police come |

Telekinesis, Force and Lightning Strike all use the mouse buttons, so only one of them can be on at a time. Powers run on a shared **energy bar** that drains and refills; you can switch it off in the menu for infinite energy.

### CLEO scripts (learning projects)
| Script | Key | What it does |
|---|---|---|
| `hello` | F5 | Help box with a press counter |
| `vehicle_spawner` | F7/F8 choose, F6 spawn, Shift+F6 spawn and enter | 12 vehicles, from the Infernus to the Hydra |
| `console` | F9 | Command console: `car infernus`, `weapon minigun`, `money 1000000`, `wanted 0`, `health`, `fix`, `time 12`, `weather 8`, `tp marker`, `save home` / `tp home`, toggles `god` / `ammo` / `cops` / `stamina` |

Details: [cleo/README.md](cleo/README.md).

## Requirements
Everything is free except the game. Download only from these official pages.

### The game and the in-game mods
| What | Version used | Why | Download |
|---|---|---|---|
| GTA San Andreas | Steam build, **downgraded to 1.0 US** in a copy | Almost every mod and tool needs the original 1.0 US `gta_sa.exe` | [Steam store page](https://store.steampowered.com/app/12120/) |
| San Andreas Downgrader | - | Turns the copy into v1.0 US | [GTAForums topic](https://gtaforums.com/topic/927016-san-andreas-downgrader/) (what we used) or [Jetpack Downgrader](https://github.com/Zalexanninev15/Jetpack-Downgrader/releases) (open source) |
| CLEO 5 + Ultimate ASI Loader | 5.4.0 | Runs `.cs` scripts and loads `.asi` plugins | [Release v5.4.0](https://github.com/cleolibrary/CLEO5/releases/tag/v5.4.0) → `SA.CLEO-v5.4.0+Ultimate_ASI_Loader.zip` · [cleo.li](https://cleo.li/) |
| SilentPatch | 1.1 build 34.1 | Bug fixes and modern Windows compatibility | [Release 1.1-BUILD34.1-SA](https://github.com/CookiePLMonster/SilentPatch/releases/tag/1.1-BUILD34.1-SA) → `SilentPatchSA.zip` |
| Modloader | 0.3.10 | Mods live in their own folders; the game files stay clean | [Release v0.3.10](https://github.com/thelink2012/modloader/releases/tag/v0.3.10) → `modloader.zip` |
| Widescreen Fix *(optional)* | - | Proper widescreen and HUD proportions | [Release gtasa](https://github.com/ThirteenAG/WidescreenFixesPack/releases/tag/gtasa) → `GTASA.WidescreenFix.zip` |

### Development tools
| What | Version used | Why | Download |
|---|---|---|---|
| Git | 2.56 | Version control | [git-scm.com/download/win](https://git-scm.com/download/win) |
| VSCode | latest | Editor, build task (Ctrl+Shift+B) | [code.visualstudio.com](https://code.visualstudio.com/) |
| C/C++ extension | latest | IntelliSense for plugin-sdk | [ms-vscode.cpptools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools) |
| Visual Studio 2022 Build Tools | 17.14 (MSVC 14.44) | C++ compiler (x86). Workload: *Desktop development with C++* | [Direct installer](https://aka.ms/vs/17/release/vs_BuildTools.exe) · [downloads page](https://visualstudio.microsoft.com/downloads/) (*Tools for Visual Studio*) · or `winget`, see [setup](docs/setup.md#5-dev-tools) |
| Sanny Builder | 4.2.0 | Compiles CLEO scripts | [Release v4.2.0](https://github.com/sannybuilder/dev/releases/tag/v4.2.0) → `SannyBuilder-v4.2.0.zip` · [sannybuilder.com](https://sannybuilder.com/) |
| plugin-sdk | commit `15f15b6` | GTA SA engine headers and library | [github.com/DK22Pac/plugin-sdk](https://github.com/DK22Pac/plugin-sdk) (clone it, see [setup](docs/setup.md#6-plugin-sdk)) |
| Windows | 10/11 | | |

## Getting started
The full setup, with download links and checks for each step, is in **[docs/setup.md](docs/setup.md)**. In short:

1. **Copy** the Steam game folder to `C:\Games\GTA-SA-Modding`.
2. **Downgrade** that copy to v1.0 US. `gta_sa.exe` must be 14,383,616 bytes.
3. **Install** CLEO 5, SilentPatch and Modloader into the copy. Launch it once: the CLEO version shows in the main menu.
4. **Back up** the clean modded copy (`C:\Games\GTA-SA-Modding-clean.zip`).
5. **Install the dev tools**: Sanny Builder, VS 2022 Build Tools with the C++ workload, and plugin-sdk (cloned to `C:\dev\plugin-sdk`, built, with `PLUGIN_SDK_DIR` set).
6. **Clone this repo** to `C:\dev\gta-sa-mods` and run:
   ```powershell
   .\scripts\run.ps1
   ```

## Day-to-day workflow
```powershell
.\scripts\run.ps1                    # build the C++ plugin, deploy everything, start the game
.\scripts\run.ps1 -NoBuild           # deploy + start (e.g. after compiling only CLEO scripts)
.\scripts\build.ps1                  # build only (Release)
.\scripts\build.ps1 -Configuration Debug
.\scripts\deploy.ps1                 # copy built mods + config into the game
```
- **C++:** edit `plugins/ArmageddonPlus/src/...`, then run `run.ps1`. In VSCode, **Ctrl+Shift+B** builds.
- **CLEO:** edit `cleo/*.txt` in Sanny Builder, press **F6** to compile, then run `run.ps1 -NoBuild`.
- **Settings:** edit `config/ArmageddonPlus.ini`, then `deploy.ps1` and restart the game.
- **Close the game before building or deploying.** Windows locks a loaded `.asi`, and `run.ps1` refuses to start while the game is running.

Everything is deployed under `C:\Games\GTA-SA-Modding\modloader\`:
- `ArmageddonPlus\`: `ArmageddonPlus.asi` and `ArmageddonPlus.ini`
- `MyCleoMods\`: the compiled `.cs` scripts

To disable a mod, delete its folder or rename it to start with `.`.

Logs, when something goes wrong: `ArmageddonPlus.log`, `cleo.log` and `modloader\modloader.log` in the game folder. See [docs/troubleshooting.md](docs/troubleshooting.md).

## Project structure
```
gta-sa-mods/
├── cleo/                        CLEO scripts (Sanny Builder .txt sources; compiled .cs are git-ignored)
├── config/
│   └── ArmageddonPlus.ini       power settings (levels, speeds, keys); deployed next to the .asi
├── docs/
│   ├── setup.md                 environment setup, from a clean PC to a working modded game
│   ├── how-a-mod-works.md       deep dive: from C++ to the game (HUD + Flight as examples)
│   ├── powers.md                player guide: every power, controls, tips
│   └── troubleshooting.md       known problems and fixes
├── plugins/ArmageddonPlus/      C++ plugin (plugin-sdk)
│   ├── ArmageddonPlus.vcxproj   MSBuild project (compiles every src/**/*.cpp)
│   └── src/
│       ├── main.cpp             entry point: registers powers, hooks game events
│       ├── core/                Config, Input, Log, Power, PowerManager, Menu, Energy, Fx, Sound
│       ├── powers/              one class per superpower
│       └── world/               world mods: Gravity, VehicleHandling, Chaos, Reactions
├── scripts/                     build.ps1, deploy.ps1, run.ps1
└── .vscode/                     build tasks + IntelliSense paths for plugin-sdk
```
How the plugin is built, and how to add a power: [plugins/ArmageddonPlus/README.md](plugins/ArmageddonPlus/README.md).

## Configuration
All power tuning lives in [config/ArmageddonPlus.ini](config/ArmageddonPlus.ini). It has one `[Power.<Name>]` section per power, with per-level values like `Speed1..3`, `Multiplier1..3` and `ThrowSpeed1..3`. Every key has a comment.

- **Menu key:** `[Menu] Key=F10`. **Never use F1:** it makes GTA SA stop reading the keyboard.
- **Optional hotkeys:** add `Key=<key>` to a power's section, e.g. `Key=NUMPAD1`. A toggle hotkey steps off → 1 → 2 → 3 → off.
- **Top-right power list:** `[Debug] Overlay=1`, also switchable in the menu.

## Roadmap
- [x] **Phase 0: Environment.** Game copy, 1.0 US downgrade, CLEO / SilentPatch / Modloader, dev tools.
- [x] **Phase 1: CLEO.** Hello world, vehicle spawner, command console, quality-of-life toggles.
- [x] **Phase 2: C++ foundation.** plugin-sdk, hello `.asi`, power framework (config, input, levels, overlay).
- [x] **Phase 3: Powers.** Super speed + jump, explosion punch, flight, telekinesis, force push/pull, bullet time, lightning strike, fire trail.
- [x] **Phase 4: Power UX.**
  - [x] In-game power menu
  - [x] Energy bar with drain / costs / regen (switchable in the menu)
  - [x] Visual effects (game particles) and synthesized power sounds
- [x] **Phase 5: Physics and world.** Gravity modes, vehicle handling, chaos modes, NPC and police reactions.

## Documentation
| Doc | For |
|---|---|
| [docs/how-a-mod-works.md](docs/how-a-mod-works.md) | **Deep dive:** how a GTA SA mod works from code to game. The ASI loader, plugin-sdk hooks, the frame loop, then the HUD/menu and Flight step by step |
| [docs/setup.md](docs/setup.md) | Setting up a PC from scratch |
| [docs/powers.md](docs/powers.md) | Playing: what each power does and how to control it |
| [docs/troubleshooting.md](docs/troubleshooting.md) | Things that went wrong before, and their fixes |
| [plugins/ArmageddonPlus/README.md](plugins/ArmageddonPlus/README.md) | C++ plugin internals, adding a power, plugin-sdk tips |
| [cleo/README.md](cleo/README.md) | CLEO scripts, Sanny Builder workflow, CLEO cheat sheet |

## Credits
- [plugin-sdk](https://github.com/DK22Pac/plugin-sdk) (DK22Pac and the GTA community). `SpawnVehicle` adapts its `CreateCar` example.
- [CLEO 5](https://github.com/cleolibrary/CLEO5) (cleolibrary), [Sanny Builder](https://github.com/sannybuilder/dev) (Seemann and contributors).
- [SilentPatch](https://github.com/CookiePLMonster/SilentPatch) (Silent / CookiePLMonster), [Modloader](https://github.com/thelink2012/modloader) (LINK/2012), [Ultimate ASI Loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader) (ThirteenAG).
- Inspired by the *GTA San Andreas Armageddon* mod.

Grand Theft Auto: San Andreas is © Rockstar Games. This repository only contains original source code and configuration, **no game files**. You need your own legal copy of the game.
