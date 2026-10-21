# Phase 0: Environment setup

Goal: a separate, modded copy of the game on **v1.0 US** with an ASI loader, CLEO, SilentPatch and Modloader, plus the dev tools.

> **Safety:** GTA SA is single-player and has no anti-cheat, so you won't get a Steam ban. We still never touch the Steam folder: if anything breaks, Steam's "Verify integrity of game files" restores it. Only download tools from the sources listed here (the official GitHub/author pages).

## Status
- [x] 1. Copy Steam game -> `C:\Games\GTA-SA-Modding` (done 2026-10-21, 416 files, 4.6 GB)
- [ ] 2. Downgrade the copy to v1.0 US
- [ ] 3. Install base mods
- [ ] 4. First launch + backup
- [ ] 5. Install dev tools
- [x] 6. Repo + deploy scripts

## 2. Downgrade to 1.0 US
Your Steam build is the newer one (`gta-sa.exe`, about 5.9 MB). CLEO, plugin-sdk and almost every mod need the original `gta_sa.exe` 1.0 US (14,383,616 bytes).

- Use a "GTA SA Downgrader" that supports the **current Steam** release. Before running anything we'll agree together on the exact download link. Downgraders are community tools and come from forums and mod sites, not GitHub.
- Run it **only on `C:\Games\GTA-SA-Modding`**, never on the Steam folder.
- Pick the options that restore v1.0 US. The extras (radio songs, original audio) are optional.
- Check the result: `C:\Games\GTA-SA-Modding\gta_sa.exe` exists and is 14,383,616 bytes:
  ```powershell
  (Get-Item C:\Games\GTA-SA-Modding\gta_sa.exe).Length
  ```
- Saves: v1.0 might not read saves from the Steam version. Start a fresh game or use a 100% save made for 1.0. Saves are stored in `Documents\GTA San Andreas User Files`.

## 3. Base mods (all extracted into `C:\Games\GTA-SA-Modding`)
| Mod | Why | Source |
|---|---|---|
| CLEO 5 (5.4.0+) | Script runtime. The release package already includes an ASI loader. | https://github.com/cleolibrary/CLEO5/releases |
| Ultimate ASI Loader | Loads `.asi` plugins (skip it if CLEO's package already includes it) | https://github.com/ThirteenAG/Ultimate-ASI-Loader/releases |
| SilentPatch | Bug fixes and modern-Windows compatibility | https://github.com/CookiePLMonster/SilentPatch/releases |
| Modloader | One folder per mod, so the game files stay clean | https://github.com/thelink2012/modloader/releases |
| *(optional)* Widescreen fix | Proper widescreen/HUD | https://github.com/ThirteenAG/WidescreenFixesPack/releases |

Install order: CLEO 5 -> SilentPatch -> Modloader. Each one's readme lists the files to copy. They usually go into the game root.

## 4. First launch
1. Run `C:\Games\GTA-SA-Modding\gta_sa.exe` directly.
2. Check that CLEO's version text appears in the main menu.
3. Quit and zip the folder as `C:\Games\GTA-SA-Modding-clean.zip` (a "known good" restore point).

## 5. Dev tools
| Tool | Use | Source |
|---|---|---|
| Sanny Builder 4 | Write and compile CLEO scripts (`.txt` -> `.cs`) | https://sannybuilder.com |
| Visual Studio 2022 Build Tools | C++ compiler (MSVC, x86). Workload: **Desktop development with C++** | https://visualstudio.microsoft.com/downloads/ (Tools for Visual Studio) |
| plugin-sdk | GTA SA engine headers and lib | https://github.com/DK22Pac/plugin-sdk (clone to `C:\dev\plugin-sdk`) |
| VSCode C/C++ extension | IntelliSense | VSCode marketplace (`ms-vscode.cpptools`) |

We'll set up plugin-sdk (build it, environment variable) together at the start of Phase 2, following its wiki.
