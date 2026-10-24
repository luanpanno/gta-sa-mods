# ArmageddonPlus: power guide

How to use every power in game. Tuning values live in [config/ArmageddonPlus.ini](../config/ArmageddonPlus.ini). After editing, run `.\scripts\deploy.ps1` and restart the game.

## The menu
Press **F10** to open the power menu. While it's open, CJ stands still, and powers that are already on keep working.

| Key | Action |
|---|---|
| ↑ / ↓ | Select a power (hold to scroll) |
| ← / → | Lower / raise the level: Off ↔ 1 ↔ 2 ↔ 3 |
| Enter | Toggle the selected power on/off (back on at its last level), or fire a one-shot power (Spawn Vehicle) |
| Insert (or B) | **Bind a shortcut** to the selected power: the menu says "press a key...", and the next key you press becomes its shortcut |
| Delete | Remove the selected power's shortcut |
| C | **Customize** the selected power (see below) |
| Backspace or F10 | Close (while binding: cancel) |

Each row shows a green dot when the power is on, its shortcut as a key badge (e.g. `[X]`), and its level as pips (■■□) or a level name ("Moon"). The bottom of the menu describes the selected power and lists these keys.

The bottom of the menu also has:
- **Energy system:** On/Off (off by default). Off means infinite energy.
- **HUD power list:** the list in the top-right corner.
- **Fog:** Off = no distance fog, clear view up to the draw distance (also clears the haze in foggy weather). Saved like the other settings.
- **Close menu**.

## Customize
Select a power in the menu and press **C** to open its Customize page: every tuning value (speed, height, radius, force...) as a slider, from its minimum up to the **max**, the most the game handles well. A full gold bar means the value is at the max.

| Key | Action |
|---|---|
| ↑ / ↓ | Select a value |
| ← / → | Lower / raise it (hold Shift: 10x bigger steps) |
| Home / End | Jump to the minimum / the max |
| 0-9 | Type an exact value, then **Enter** (`.` or `,` for decimals; Backspace deletes; out-of-range values are clamped) |
| Backspace | Back to the power list |

- **Editing level** (top row): each level has its own values. Pick the level, then change its values. Settings marked "every level" (e.g. Flight's responsiveness) are shared.
- Changes apply right away, even while the power is on, and are saved to `ArmageddonPlus.user.ini`.
- **Reset to defaults** puts back the values from `ArmageddonPlus.ini`.

| Power | What you can customize (max) |
|---|---|
| Flight | Speed (250 m/s), up/down speed (150 m/s), responsiveness |
| Super Speed | Run speed (30x), leg animation, swim speed (20x), speed kept in jumps (4x) |
| Super Jump | Height (400 m), distance (8x), ground pound radius (60 m) and force |
| Explosion Punch | Victim launch speed, ring explosions (20), ring radius, shockwave radius (80 m) and force |
| Telekinesis | Throw speed (250 m/s), grab range (300 m) |
| Force Push / Pull | Radius (80 m), push and pull force |
| Lightning Strike | Bolts per strike (25), spread, shockwave radius (100 m) and force |
| Fire Trail | Flame duration (20 s) |
| Bullet Time | Game speed (5% = slowest) |

## Shortcuts
Shortcuts let you switch powers on and off without opening the menu. They're most useful for the "always on" kind, like Flight, Super Speed or Bullet Time.

1. Open the menu (F10), select the power, and press **Insert**.
2. Press the key you want, e.g. **X**. The footer confirms "Flight: X".
3. Close the menu. Now each press of **X** steps Flight up: **off → level 1 → level 2 → level 3 → off**. Powers without levels just toggle on/off.

- One key can only belong to one power. Binding a key that's already in use moves it, and the footer tells you from which power.
- You can't bind keys the menu uses (arrows, Enter, Insert, Delete, F10), mouse buttons or **F1**. F1 makes GTA SA stop reading the keyboard.
- Avoid keys the game uses on foot or in cars (W/A/S/D, F, Space, C, Tab, Q/E...). Good picks: X, Z, V, G, the number keys, the numpad.
- Shortcuts, the menu settings and Customize values are saved to `C:\Games\GTA-SA-Modding\ArmageddonPlus.user.ini`. Deploys never overwrite it, and you can delete it to reset everything.

## The HUD
- Everything hides during cutscenes and scripted widescreen scenes, and whenever a mission hides the game's HUD.
- **Top-right:** the **Armageddon+** title and the power list below.
- **Energy bar** (bottom center, when the energy system is on; also visible while the menu is open): it turns orange below 25%, and flashes red when you try something you can't afford.
- **Power list:** powers that are **on** (bright, with their level) and powers that have a **shortcut** (dimmed, with their key badge). Powers with neither are hidden, to keep the screen clean. You can switch the list off in the menu.
- The top-right part hides while the menu is open, because the menu shows the same information.
- Everything (menu and HUD) scales with `[Menu] Scale` in the ini: 1.0 is the default, and you can try 1.2 on a big screen.

## Energy
**Off by default** (infinite energy, no bar). Turn **Energy system** on in the menu, or set `[Energy] Enabled=1` in the ini, for powers to cost energy. Powers then share one **energy bar** (bottom center of the screen, 0-100).

- **Toggle powers drain it** every second while they're on, and more at higher levels. Super Speed and Fire Trail only drain while CJ is moving. When the bar hits 0, draining powers switch off ("Out of energy").
- **Actions cost a lump sum:** a punch blast, a force push/pull, a telekinesis grab, a lightning strike. Spawn tools are free. If you don't have enough, the action doesn't happen: you get an error sound and the bar flashes red.
- **It refills by itself** (8 per second) once you haven't spent anything for 1.5 seconds. Bullet Time doesn't change the drain or refill speed.
- A draining power needs at least 5 energy to switch on.
- Want it off again? Turn **Energy system** off in the menu.

Default costs (all tunable in the ini: `[Energy]`, plus `Drain1-3` / `Cost1-3` in each power's section):

| Power | Level 1 | Level 2 | Level 3 |
|---|---|---|---|
| Flight | 3 /s | 5 /s | 8 /s |
| Super Speed | 1.5 /s | 3 /s | 6 /s |
| Fire Trail | 1.5 /s | 3 /s | 5 /s |
| Bullet Time | 5 /s | 9 /s | 15 /s |
| Invulnerability | 3 /s | | |
| Super Jump | 3 per jump | 5 | 8 |
| Explosion Punch | 5 per blast | 10 | 18 |
| Telekinesis | 6 per grab | 10 | 15 |
| Force Push / Pull | 10 per blast | 16 | 25 |
| Lightning Strike | 12 per strike | 20 | 35 |

General rules:
- **All powers switch off when CJ dies,** and draining powers switch off when the energy runs out (see [Energy](#energy)).
- **Mouse powers:** Telekinesis, Force and Lightning Strike all use the mouse buttons, so turning one on turns the others off. While one is on, CJ can't shoot or punch. Turn it off to use weapons again.
- **On foot only:** most powers only work on foot. Fire Trail and Invulnerability also work in vehicles.

## Effects and sounds
| Power | What you see / hear |
|---|---|
| Flight | Whoosh on take-off, dust under CJ while he flies low (helicopter-style downdraft) |
| Super Speed | Dust burst when he lands from a jump |
| Force | Whoosh out (push) or in (pull), plus a dust shockwave in front of CJ |
| Telekinesis | Electric zap and a flash on the grabbed thing, whoosh on throws |
| Lightning Strike | Thunder (louder at higher levels), with the bolt and screen flash |
| Bullet Time | Deep "slow down" sound on, "speed up" sound off |

- The dust and flashes are the game's own particle effects. The sounds are generated by the plugin at startup, so there are no audio files to install.
- **Use your own sounds:** put WAV files named `whoosh`, `whoosh_in`, `thunder`, `zap`, `slowdown` or `speedup` (`.wav`) in `C:\Games\GTA-SA-Modding\modloader\ArmageddonPlus\sounds\`. Each one replaces the generated sound with the same name.
- ini `[Effects]`: `Particles=0/1`, `Sounds=0/1`, `Volume` (0-1).

---

## Flight
Superman-style flight. CJ dives head-first while moving and treads in the air while hovering.

| Key | Action |
|---|---|
| W / S | Fly toward / away from where the camera looks. Look up + W climbs. |
| A / D | Strafe |
| Space / C | Straight up / down |
| No keys | Hover in place |

| Level | Speed | Up/down |
|---|---|---|
| 1 Cruise | 20 m/s | 12 m/s |
| 2 Fast | 55 m/s | 30 m/s |
| 3 Very fast | 110 m/s (~400 km/h) | 60 m/s |

- He can't fly through buildings: the game's own collisions apply.
- Climbing reaches the full Up/down speed (and the full Speed when diving upward): the game slows peds that rise, so Flight makes up the difference each frame.
- He takes no crash or fall damage while flying, and that protection lasts until he next touches the ground. Turning Flight off mid-air is safe.
- Getting into a vehicle stops flight.
- ini: `Speed1-3`, `Climb1-3`, `AscendKey`, `DescendKey`, `Responsiveness` (higher = snappier), `FollowCameraPitch=0` (W always flies level), `Poses=0` (no Superman poses).

## Super Speed
CJ runs much faster. Just run as usual. Jumps stay normal height; for high jumps, turn on Super Jump too.

| Level | Speed | Swim speed | Speed kept in jumps |
|---|---|---|---|
| 1 Fast | 2.5x | 2x | 1.2x |
| 2 Faster | 4.5x | 3.5x | 1.35x |
| 3 Super fast | 8x | 6x | 1.5x |

- He never gets tired while it's on.
- Jumps don't slow him down: he keeps his run speed in the air, boosted per level (table above), so running jumps are long. He lands on his feet (no fall-over / get-up animation).
- He stops before walls instead of passing through. Pedestrians get shoved aside.
- While Flight is on, Super Speed stands down.
- **Walks on water:** on foot, he stands, walks and runs on lakes and the sea (an invisible platform under his feet). If he's already swimming when you turn it on, he swims faster instead, on the surface and underwater. `WaterWalk=0` turns water walking off.
- ini: `Multiplier1-3`, `AnimSpeed1-3` (leg speed, visual only), `SwimMultiplier1-3`, `WaterWalk`, `JumpForward1-3` (run speed kept in the air).

## Super Jump
Super jumps without needing Super Speed. Just jump as usual: CJ goes much higher and further, and takes no fall damage until he lands. He slows down in the last few meters and lands on his feet, without tripping.

| Level | Height | Distance | Landing |
|---|---|---|---|
| High | ~30 m | 1.5x | dust |
| Higher | ~83 m | 2.2x | small ground pound (10 m) |
| Sky | ~130 m | 3x | **ground pound**: a 20 m shockwave throws everything around you |

- With Super Speed also on, Super Jump sets the height, and Super Speed's run speed carries through the whole jump with no air slowdown. Expect huge running jumps.
- Under Gravity Moon, jumps go even higher.
- Costs 3 / 5 / 8 energy per jump. With too little energy, CJ just does a normal jump.
- ini: `Jump1-3` (take-off speed), `Forward1-3` (distance multiplier), `PoundRadius1-3`, `PoundSpeed1-3`, `Cost1-3`.

## Explosion Punch
Every melee hit (fists, bat, katana...) **launches the victim** away from CJ, blows it up, sets off a **ring of explosions** around it and sends out a **shockwave** that throws nearby people, cars and objects. Punching a car does the same to the car.

| Level | Each punch |
|---|---|
| 1 | Car-bomb blast, 3-blast ring (5 m), 18 m shockwave at 30 m/s, victim launched at 40 m/s |
| 2 | Aircraft-size blast, 6-blast ring (9 m), 30 m shockwave at 50 m/s, victim launched at 65 m/s |
| 3 | Aircraft-size blast, 10 aircraft-size blasts in a 14 m ring, 45 m shockwave at 80 m/s, victim launched at 100 m/s |

- CJ is immune to explosions while it's on, including enemy rockets.
- Guns don't trigger it.
- ini: `RingBlasts1-3`, `RingRadius1-3`, `ShockRadius1-3`, `ShockSpeed1-3`, `Launch1-3`, `Cooldown` (one swing = one blast).

## Telekinesis
*Mouse power.* CJ draws a Desert Eagle that can't fire; it's only there for the crosshair.

| Input | Action |
|---|---|
| Hold RMB | Aim |
| RMB + click LMB | Grab whatever is under the crosshair (car, person, object). Aiming at a driver grabs the whole car. |
| Keep LMB held | It floats in front of you and follows your aim |
| Release LMB | Throw it where you're aiming |

| Level | Throw speed |
|---|---|
| 1 | 40 m/s |
| 2 | 80 m/s |
| 3 | 140 m/s |

- Buildings can't be grabbed. Lamp posts and other fixed props may not move.
- The weapon CJ had in the pistol slot comes back when you turn Telekinesis off.
- ini: `ThrowSpeed1-3`, `Range` (grab distance), `AimWeapon` (24 Desert Eagle, 22 Pistol, 41 Spraycan).

## Force Push / Pull
*Mouse power.* Affects everything in a wide cone in front of CJ: people, cars and objects.

| Input | Action |
|---|---|
| LMB | **Push**: shockwave that sends things flying away |
| RMB | **Pull**: yanks things toward CJ |

| Level | Radius | Push | Pull |
|---|---|---|---|
| 1 | 14 m | 25 m/s | 16 m/s |
| 2 | 24 m | 45 m/s | 26 m/s |
| 3 | 40 m | 80 m/s | 40 m/s |

- ini: `Radius1-3`, `PushSpeed1-3`, `PullSpeed1-3`, `Cooldown`.

## Lightning Strike
*Mouse power.* A small blue crosshair appears in the middle of the screen. Point the camera at a spot and click **LMB**: a lightning bolt hits it.

| Level | Strike |
|---|---|
| 1 | 2 bolts within 6 m, car-bomb blasts + fire, 22 m shockwave at 40 m/s |
| 2 | 5 bolts within 14 m, aircraft-size main blast + fire, 35 m shockwave at 65 m/s |
| 3 | **Storm:** 10 bolts within 24 m, all aircraft-size blasts, fire everywhere, 55 m shockwave at 100 m/s |

- Pointing at the sky does nothing, because there's nothing to hit.
- CJ is explosion-proof while Lightning Strike is on, so striking right next to yourself is safe.
- ini: `Bolts1-3`, `Spread1-3`, `ShockRadius1-3`, `ShockSpeed1-3`, `Range`, `Cooldown`.
- Bullet Time doesn't slow down the bolt animation.

## Fire Trail
CJ leaves a trail of fire wherever he runs. In a car, the flames appear behind the car. He and his current car are fireproof.

| Level | Flames last | Spreading |
|---|---|---|
| 1 | 2 s | no |
| 2 | 4 s | no |
| 3 | 8 s | yes |

- No flames while airborne (jumping or flying).
- The game limits how many fires can burn at once, so very long trails cut off at the far end.
- Best combo: Super Speed level 3.
- ini: `Duration1-3`, `Spread1-3`, `Spacing`.

## Bullet Time
The whole world slows down, and CJ keeps moving at normal speed. A light blue tint shows it's on.

| Level | Game speed |
|---|---|
| 1 | 60% |
| 2 | 35% |
| 3 | 15% |

- CJ's walking, running, punching and reloading stay real-time. His jumps and falls are slow-motion, which looks cinematic.
- Flight stays real-time too.
- Sound isn't slowed down.
- Driving is slow-motion as well: CJ isn't sped up in vehicles.
- ini: `Scale1-3`, `PlayerNormalSpeed=0` (CJ slows too), `TintAlpha` (0 = no tint).

## Invulnerability
CJ, and the vehicle he's in, can't be damaged: bullets, fire, explosions, crashes or melee. His health stays full. A car he gets out of goes back to normal.

# Spawn tools
The **SPAWN** section of the menu: **←/→** choose (hold **Shift** to move 10 at a time), **Enter** uses. They don't cost energy.

## Spawn Vehicle
Every spawnable vehicle in the game (about 200: cars, bikes, boats, planes, helicopters, trailers), listed with its in-game name and model ID, e.g. `Infernus  411`. The list comes from the game's own data, so it's always complete. Trains and RC toys are excluded. **Enter** spawns the selected vehicle 8 m in front of CJ. This doesn't work inside interiors.
- ini: `Model` = the vehicle selected when the game starts.

## Give Weapon
Every weapon CJ can hold, from brass knuckles to the minigun, plus the parachute, night vision and so on. **Enter** gives it with 500 ammo (melee weapons: 1) and makes it CJ's current weapon. Satchel charges come with their detonator.
- ini: `Weapon` (selected at startup, weapon type ID), `Ammo`.

## Add Ammo
**←/→** picks `Current +100`, `Current +500`, `All guns +500`, `All guns +1000` or `All guns MAX` (9999), and **Enter** refills. "Current" is the weapon CJ is holding; "All guns" is every gun he carries. Melee weapons are skipped.

## Add Money
**←/→** picks $1,000 / $10,000 / $100,000 / $1,000,000 / $10,000,000, and **Enter** adds it. The HUD money counter rolls up like after a mission. It's capped at $999,999,999, the most the HUD can show.
---

# Cheats
The **Cheats** row at the bottom of the menu (it shows how many are on, e.g. `3 ON >>`) opens the **cheat page**. It lists all 91 of the game's cheats, grouped: Player, Weapons, Wanted level, Spawn vehicles, Cars & traffic, People & gangs, Weather & time. No codes to type.

| Key | Action |
|---|---|
| ↑ / ↓ | Select |
| Enter | Use a **one-time** cheat (weapon sets, health + $250k, spawning a Rhino, weather...), or switch a **toggle** cheat on/off |
| ← / → | Toggle cheats: off / on |
| Backspace | Back to the power menu (F10 closes everything) |

- Toggle cheats (infinite ammo, pink traffic, riot, mega jump...) have a green dot and **ON** while active. The page header shows how many are on.
- Active toggle cheats are also listed in the HUD under the powers ("CHEATS": up to 6, then "+N more").
- Using a cheat from here does exactly what typing its code does: the game's own cheat functions run, conflicting cheats switch each other off (pink vs. black traffic, for example), and the game marks you as having cheated, just like typing.

---

# World mods
The **WORLD** section of the menu has mods that change the world rather than CJ. They don't use energy.

## Gravity
Gravity for everything: CJ, people, cars, objects, debris.

| Level | Gravity |
|---|---|
| Moon | 17% |
| Mars | 38% |
| Zero-G | 2%: things barely come down |
| Heavy | 250% |

Combines well with Super Speed or Super Jump (huge moon jumps) and Force (cars drifting away in Zero-G).
- ini: `Moon`, `Mars`, `ZeroG`, `Heavy` (multipliers).

## Vehicle Handling
Super handling for whatever CJ is **driving**.

| Input | Action |
|---|---|
| Hold **Shift** | Turbo: extra acceleration up to a higher top speed |
| **J** | Jump the car (wheels on the ground) |
| Always | More grip in corners, tyres can't burst, the car flips back onto its wheels if it lands upside down and stops |

| Level | Turbo | Top speed | Jump |
|---|---|---|---|
| Sport | 12 m/s² | 55 m/s (~200 km/h) | 6 m/s |
| Race | 25 m/s² | 85 m/s (~300 km/h) | 9 m/s |
| Rocket | 50 m/s² | 140 m/s (~500 km/h) | 14 m/s |

- ini: `TurboKey`, `JumpKey`, `TurboAccel1-3`, `TopSpeed1-3`, `Grip1-3`, `Jump1-3`.

## Chaos
One chaos mode at a time:

| Level | Mode |
|---|---|
| Meteors | Explosions rain down at random spots 12-60 m around CJ |
| Exploding Traffic | A random nearby car blows up every 2.5 s |
| Flying Cars | Random nearby cars get launched into the air, spinning |
| Riot | Everyone fights everyone (the game's own riot cheat) |

Mission vehicles are never touched, so missions keep working.
- ini: `MeteorInterval`, `ExplodeInterval`, `LaunchInterval`, `Radius`.

## NPC Reactions
How the world reacts **while any superpower is on**:

| Level | Reaction |
|---|---|
| Fear | Pedestrians within 35 m run away from CJ, screaming |
| Fear + Police | ...and the wanted level goes up to at least 2 stars |

Mission characters aren't affected. Note that Invulnerability counts as a superpower too.
- ini: `Radius`, `PoliceStars`.

---

## Fun combos
- **Flash:** Super Speed 3 + Fire Trail 2.
- **Superman:** Flight 2 + Invulnerability + Force (push from the air).
- **Max Payne:** Bullet Time 2 + Explosion Punch 1.
- **Storm god:** Flight 1 (hover high) + Lightning Strike 3.
- **Moonwalker:** Gravity Moon + Super Speed 2 + Super Jump 1: jumps over buildings.
- **Armageddon:** Chaos Meteors + Invulnerability + Bullet Time 1.
- **Stunt driver:** Vehicle Handling Rocket + Gravity Mars + jump off a ramp.
