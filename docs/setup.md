# Setup: from a clean PC to a working modded game

The result: a **separate, modded copy** of GTA San Andreas on **v1.0 US**, with CLEO 5, SilentPatch and Modloader, plus the tools to build this repo.

> **Safety:** GTA SA is single-player and has no anti-cheat, so there's no Steam ban risk. We still never touch the Steam folder: if anything breaks, Steam's "Verify integrity of game files" restores it. Only download tools from the sources listed here (the official GitHub/author pages).

| Path | What |
|---|---|
| `C:\Program Files (x86)\Steam\steamapps\common\Grand Theft Auto San Andreas` | Steam install. **Never modified.** |
| `C:\Games\GTA-SA-Modding` | Modded copy. Launch `gta_sa.exe` from here, **not via Steam**. |
| `C:\Games\GTA-SA-Modding-clean.zip` | Restore point: clean modded copy |
| `C:\Tools\SannyBuilder` | Sanny Builder (portable) |
| `C:\dev\plugin-sdk` | plugin-sdk (`PLUGIN_SDK_DIR`) |
| `C:\dev\gta-sa-mods` | This repo |

## 1. Copy the game
```powershell
robocopy "C:\Program Files (x86)\Steam\steamapps\common\Grand Theft Auto San Andreas" "C:\Games\GTA-SA-Modding" /E
```
The copy is outside Program Files, so there are no permission (UAC) problems when writing to it.

## 2. Downgrade the copy to 1.0 US
The current Steam build (`gta-sa.exe`, about 5.9 MB) isn't supported by CLEO, plugin-sdk or most mods. They need the original `gta_sa.exe` 1.0 US.

1. Use a downgrader that supports the current Steam build:
   - **What we used:** the "San Andreas Downgrader" from [GTAForums topic 927016](https://gtaforums.com/topic/927016-san-andreas-downgrader/).
   - **Open-source alternative:** [Jetpack Downgrader](https://github.com/Zalexanninev15/Jetpack-Downgrader) → [release 2.3.0.1](https://github.com/Zalexanninev15/Jetpack-Downgrader/releases/tag/2.3.0.1), `JPD_WithPatches.zip`. Needs .NET Framework 4.8, which Windows 11 already includes. Skip its "install mods" stage.
2. Point it **only at `C:\Games\GTA-SA-Modding`**.
3. Check the result:
   ```powershell
   (Get-Item C:\Games\GTA-SA-Modding\gta_sa.exe).Length   # must be 14383616
   ```
4. From now on, launch **`gta_sa.exe`** (with an underscore) only.

The downgrader also restores the 1.0 data files (`gta3.img`, `anim.img`, `main.scm`, the radio streams with the removed songs...). It keeps the originals as `*.sabackup`, about 1.8 GB, which you can delete once the game works.

**Saves:** v1.0 might not read saves from the newer Steam build. Start a new game, or use a save made for 1.0. Saves are in `Documents\GTA San Andreas User Files`.

## 3. Install the base mods
Extract each one into `C:\Games\GTA-SA-Modding`:

| Mod | Download (version used) | Adds | All releases |
|---|---|---|---|
| CLEO 5 + Ultimate ASI Loader | [SA.CLEO-v5.4.0+Ultimate_ASI_Loader.zip](https://github.com/cleolibrary/CLEO5/releases/download/v5.4.0/SA.CLEO-v5.4.0%2BUltimate_ASI_Loader.zip) | `CLEO.asi`, `bass.dll`, `cleo\`, `vorbisFile.dll` (the ASI loader) | [cleolibrary/CLEO5](https://github.com/cleolibrary/CLEO5/releases) |
| SilentPatch | [SilentPatchSA.zip](https://github.com/CookiePLMonster/SilentPatch/releases/download/1.1-BUILD34.1-SA/SilentPatchSA.zip) (1.1 build 34.1) | `SilentPatchSA.asi`, `SilentPatchSA.ini` | [CookiePLMonster/SilentPatch](https://github.com/CookiePLMonster/SilentPatch/releases) |
| Modloader | [modloader.zip](https://github.com/thelink2012/modloader/releases/download/v0.3.10/modloader.zip) (v0.3.10) | `modloader.asi`, `modloader\` | [thelink2012/modloader](https://github.com/thelink2012/modloader/releases) |
| *(optional)* Widescreen Fix | [Release gtasa](https://github.com/ThirteenAG/WidescreenFixesPack/releases/tag/gtasa) → `GTASA.WidescreenFix.zip` | widescreen and HUD fixes | [ThirteenAG/WidescreenFixesPack](https://github.com/ThirteenAG/WidescreenFixesPack/releases) |

CLEO's bundle already includes the [Ultimate ASI Loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader/releases), so you don't need to download it separately.

- **Back up the original `vorbisFile.dll` first** (e.g. as `vorbisFile.dll.original`). The ASI loader replaces it.
- To uninstall: delete the files above and rename `vorbisFile.dll.original` back.

## 4. First launch and backup
1. Run `C:\Games\GTA-SA-Modding\gta_sa.exe`. CLEO's version text should appear in the bottom-left corner of the main menu.
2. Quit, then make the restore point, leaving out the `*.sabackup` files:
   ```powershell
   cd C:\Games
   tar.exe -a -c -f GTA-SA-Modding-clean.zip --exclude "*.sabackup" GTA-SA-Modding
   ```
   To restore: delete `C:\Games\GTA-SA-Modding` and extract the zip in its place.

## 5. Dev tools
| Tool | Install | Used for |
|---|---|---|
| Git | [git-scm.com/download/win](https://git-scm.com/download/win) | this repo |
| VSCode | [code.visualstudio.com](https://code.visualstudio.com/) | editing, build task |
| C/C++ extension | [ms-vscode.cpptools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools), or in VSCode: Extensions → "C/C++" | IntelliSense |
| Sanny Builder 4 | Portable: [SannyBuilder-v4.2.0.zip](https://github.com/sannybuilder/dev/releases/download/v4.2.0/SannyBuilder-v4.2.0.zip) ([all releases](https://github.com/sannybuilder/dev/releases)), extracted to `C:\Tools\SannyBuilder` | compiling CLEO scripts |
| Visual Studio 2022 Build Tools | [vs_BuildTools.exe](https://aka.ms/vs/17/release/vs_BuildTools.exe) (pick *Desktop development with C++*), or the `winget` command below | C++ compiler (MSVC, x86) |

```powershell
winget install --id Microsoft.VisualStudio.2022.BuildTools --exact --override "--passive --wait --norestart --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended"
```
This takes 6-8 GB and 10-30 minutes, and asks for admin rights (UAC).

**Sanny Builder first run:** Game = GTA San Andreas, Edit mode = `GTA SA (v1.0 - SBL)`, Game directory = `C:\Games\GTA-SA-Modding`.

## 6. plugin-sdk
1. Clone it:
   ```powershell
   git clone --depth 1 https://github.com/DK22Pac/plugin-sdk.git C:\dev\plugin-sdk
   ```
2. Set the user environment variable, then **restart VSCode and terminals**:
   ```powershell
   [Environment]::SetEnvironmentVariable('PLUGIN_SDK_DIR', 'C:\dev\plugin-sdk', 'User')
   ```
3. Generate **VS 2022** projects. The SDK's own `tools\generate\Visual Studio.bat` targets VS 2026:
   ```powershell
   cd C:\dev\plugin-sdk
   .\tools\premake\premake5.exe vs2022 --file=tools\premake\premake5.lua
   ```
4. Build the San Andreas library, Release and Debug:
   ```powershell
   $msbuild = "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe"
   & $msbuild plugin.sln /p:Configuration=Release /p:Platform="Mixed Platforms" /target:plugin_sa /m
   & $msbuild plugin.sln /p:Configuration=zDebug  /p:Platform="Mixed Platforms" /target:plugin_sa /m
   ```
   Output: `C:\dev\plugin-sdk\output\lib\Plugin.lib` and `Plugin_d.lib`.
5. After updating plugin-sdk (`git pull`), repeat steps 3 and 4.

## 7. This repo
```powershell
git clone https://github.com/luanpanno/gta-sa-mods.git C:\dev\gta-sa-mods
cd C:\dev\gta-sa-mods
.\scripts\run.ps1        # builds the plugin, deploys everything, starts the game
```
Then compile the CLEO scripts once in Sanny Builder (open each `cleo\*.txt` and press F6), and run `.\scripts\run.ps1 -NoBuild`.

Check in game:
- "ArmageddonPlus" appears in the top-right corner.
- **F10** opens the power menu.
- **F9** opens the CLEO console.

---

## Record: this machine (2026-10-21)
- Game copy: 416 files, 4.6 GB. Downgraded `gta_sa.exe` SHA256 starts `A559AA77`.
- CLEO 5.4.0 (with Ultimate ASI Loader), SilentPatch 1.1 build 34.1, Modloader 0.3.10.
- Backup `GTA-SA-Modding-clean.zip`: 3.9 GB.
- Sanny Builder 4.2.0, VS 2022 Build Tools 17.14 (MSVC 14.44), plugin-sdk commit `15f15b6` (Sep 2026).
