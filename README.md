# gta-sa-mods

Incremental GTA: San Andreas mods, from small CLEO scripts up to a C++ "Armageddon+" superpowers plugin.

## Folders
| Path | What |
|---|---|
| `C:\Program Files (x86)\Steam\...\Grand Theft Auto San Andreas` | Steam install. **Never modified.** |
| `C:\Games\GTA-SA-Modding` | Modded copy, downgraded to v1.0 US. Launch `gta_sa.exe` from here, **not via Steam**. |
| `C:\dev\gta-sa-mods` | This repo (source only, no game files). |

## Repo layout
```
docs/setup.md             Phase 0 setup guide + tool sources/versions
cleo/                     CLEO script sources (.txt, Sanny Builder) -> compiled .cs
plugins/ArmageddonPlus/   C++ plugin-sdk project (.asi)
config/                   .ini files deployed next to the plugin
scripts/build.ps1         build the C++ plugins (MSBuild, VS 2022 Build Tools)
scripts/deploy.ps1        copy built mods into the game's modloader folder
scripts/run.ps1           build + deploy + launch the game
```

## Daily loop
```powershell
.\scripts\run.ps1          # deploy everything and launch the game
.\scripts\deploy.ps1       # deploy only
```
Each mod is deployed to `C:\Games\GTA-SA-Modding\modloader\<ModName>\`. To disable one, rename its folder to start with `.` or delete it.

## Roadmap
- **Phase 0:** environment (see [docs/setup.md](docs/setup.md))
- **Phase 1:** CLEO mods: hello world, vehicle spawner, spawn console, QoL toggles
- **Phase 2:** C++ plugin foundation: hello `.asi`, spawn console port, PowerManager/Input/Config
- **Phase 3:** powers: super speed, super jump, explosion punch, flight, telekinesis, bullet time, extras
- **Phase 4:** power UX: selection menu, energy/cooldown (toggleable), effects and sounds
- **Phase 5:** physics/world: gravity, handling, chaos modes, NPC reactions
