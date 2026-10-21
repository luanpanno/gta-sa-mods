# Phase 0: Environment setup

Goal: a separate, modded copy of the game on **v1.0 US** with an ASI loader, CLEO, SilentPatch and Modloader, plus the dev tools.

> **Safety:** GTA SA is single-player and has no anti-cheat, so you won't get a Steam ban. We still never touch the Steam folder: if anything breaks, Steam's "Verify integrity of game files" restores it. Only download tools from the sources listed here (the official GitHub/author pages).

## Status
- [x] 1. Copy Steam game -> `C:\Games\GTA-SA-Modding` (done 2026-10-21, 416 files, 4.6 GB)
- [x] 2. Downgrade the copy to v1.0 US (done 2026-10-21, verified, launches to main menu)
- [x] 3. Install base mods (done 2026-10-21)
- [x] 4. First launch (CLEO version text shows) + backup `C:\Games\GTA-SA-Modding-clean.zip` (3.9 GB, 2026-10-21)
- [x] 5. Install dev tools (Sanny Builder 4.2.0, VS 2022 Build Tools / MSVC 14.44; plugin-sdk in Phase 2)
- [x] 6. Repo + deploy scripts

## 2. Downgrade to 1.0 US
Your Steam build is the newer one (`gta-sa.exe`, about 5.9 MB). CLEO, plugin-sdk and almost every mod need the original `gta_sa.exe` 1.0 US (14,383,616 bytes).

**Used:** the "San Andreas Downgrader" from [GTAForums topic 927016](https://gtaforums.com/topic/927016-san-andreas-downgrader/), run on `C:\Games\GTA-SA-Modding` only.
(An open-source alternative is [Jetpack Downgrader](https://github.com/Zalexanninev15/Jetpack-Downgrader), if this ever needs redoing.)

**What it changed (checked 2026-10-21):**
- `gta_sa.exe` is 14,383,616 bytes, the v1.0 US size. SHA256 `A559AA77...CBD26`. The old `gta-sa.exe` was replaced by the same v1.0 exe.
- It replaced the v1.0 versions of `anim.img`, `gta3.img`, `gta_int.img`, `main.scm`, `script.img`, `TrakLkup.dat` and the radio `audio\streams` (so the removed songs are back).
- The originals were kept as `*.sabackup`, about 1.8 GB in total. Keep them until the game is confirmed working, then they can be deleted.

From now on, launch **`gta_sa.exe`** only.

Check the exe again at any time:
```powershell
(Get-Item C:\Games\GTA-SA-Modding\gta_sa.exe).Length   # 14383616
```

Saves: there are no existing saves on this PC (`Documents\GTA San Andreas User Files` doesn't exist yet), so start a new game.

## 3. Base mods (all extracted into `C:\Games\GTA-SA-Modding`)
Installed 2026-10-21:

| Mod | Version / asset | Files in the game root | Source |
|---|---|---|---|
| CLEO 5 + Ultimate ASI Loader | v5.4.0, `SA.CLEO-v5.4.0+Ultimate_ASI_Loader.zip` | `CLEO.asi`, `bass.dll`, `cleo\`, and `vorbisFile.dll` (the ASI loader) | https://github.com/cleolibrary/CLEO5/releases |
| SilentPatch | 1.1 build 34.1, `SilentPatchSA.zip` | `SilentPatchSA.asi`, `SilentPatchSA.ini` | https://github.com/CookiePLMonster/SilentPatch/releases |
| Modloader | v0.3.10, `modloader.zip` | `modloader.asi`, `modloader\` | https://github.com/thelink2012/modloader/releases |
| *(optional, later)* Widescreen fix | | | https://github.com/ThirteenAG/WidescreenFixesPack/releases |

- The ASI loader replaces the game's `vorbisFile.dll`. The original is backed up as **`vorbisFile.dll.original`**.
- To uninstall everything: delete the files listed above and rename `vorbisFile.dll.original` back to `vorbisFile.dll`.
- Logs to check if something breaks: `cleo.log` and `modloader\modloader.log`, both in the game folder.

## 4. First launch
1. Run `C:\Games\GTA-SA-Modding\gta_sa.exe` directly.
2. Check that CLEO's version text appears in the main menu.
3. Quit and zip the folder as `C:\Games\GTA-SA-Modding-clean.zip` (a "known good" restore point). Leave out the `*.sabackup` files.
   To restore: delete `C:\Games\GTA-SA-Modding` and extract the zip in its place.

## 5. Dev tools
| Tool | Use | Installed |
|---|---|---|
| Sanny Builder 4.2.0 | Write and compile CLEO scripts (`.txt` -> `.cs`) | Portable, `C:\Tools\SannyBuilder\sanny.exe`. From [sannybuilder/dev releases](https://github.com/sannybuilder/dev/releases) |
| Visual Studio 2022 Build Tools 17.14 | C++ compiler (MSVC, x86). Workload: **Desktop development with C++** | Installed with `winget install Microsoft.VisualStudio.2022.BuildTools` and the `VCTools` workload |
| plugin-sdk | GTA SA engine headers and lib | https://github.com/DK22Pac/plugin-sdk (clone to `C:\dev\plugin-sdk`) |
| VSCode C/C++ extension | IntelliSense | VSCode marketplace (`ms-vscode.cpptools`) |

We'll set up plugin-sdk (build it, environment variable) together at the start of Phase 2, following its wiki.
