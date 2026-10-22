# ArmageddonPlus: power guide

How to use every power in game. Tuning values live in [config/ArmageddonPlus.ini](../config/ArmageddonPlus.ini). After editing, run `.\scripts\deploy.ps1` and restart the game.

## The menu
Press **F10** to open the power menu. While it's open, CJ stands still, and powers that are already on keep working.

| Key | Action |
|---|---|
| ↑ / ↓ | Select a power (hold to scroll) |
| ← / → | Lower / raise the level: Off ↔ 1 ↔ 2 ↔ 3 |
| Enter | Toggle the selected power on/off, or fire a one-shot power (Spawn Vehicle) |
| Backspace or F10 | Close |

The bottom of the menu also has **HUD power list** (the list in the top-right corner, active powers in green) and **Close**.

General rules:
- **All powers switch off when CJ dies.**
- **Mouse powers:** Telekinesis, Force and Lightning Strike all use the mouse buttons, so turning one on turns the others off. While one is on, CJ can't shoot or punch. Turn it off to use weapons again.
- **On foot only:** most powers only work on foot. Fire Trail and Invulnerability also work in vehicles.

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
| 1 Cruise | 12 m/s | 8 m/s |
| 2 Fast | 30 m/s | 18 m/s |
| 3 Very fast | 60 m/s | 35 m/s |

- He can't fly through buildings: the game's own collisions apply.
- He takes no crash or fall damage while flying, and that protection lasts until he next touches the ground. Turning Flight off mid-air is safe.
- Getting into a vehicle stops flight.
- ini: `Speed1-3`, `Climb1-3`, `AscendKey`, `DescendKey`, `Responsiveness` (higher = snappier), `FollowCameraPitch=0` (W always flies level), `Poses=0` (no Superman poses).

## Super Speed
CJ runs much faster and jumps much higher. Just run and jump as usual.

| Level | Speed | Jump height |
|---|---|---|
| 1 Fast | 2.5x | ~4 m |
| 2 Faster | 4.5x | ~10 m |
| 3 Super fast | 8x | ~22 m |

- He never gets tired while it's on.
- No fall damage from super jumps. At high levels, running jumps are also very long.
- He stops before walls instead of passing through. Pedestrians get shoved aside.
- While Flight is on, Super Speed stands down.
- ini: `Multiplier1-3`, `AnimSpeed1-3` (leg speed, visual only), `Jump1-3` (take-off speed; 0 = normal jump).

## Explosion Punch
Every melee hit (fists, bat, katana...) makes the victim explode. Punching a car blows the car up.

| Level | Blast |
|---|---|
| 1 | Grenade |
| 2 | Rocket |
| 3 | Car bomb + camera shake |

- CJ is immune to explosions while it's on, including enemy rockets.
- Guns don't trigger it.
- ini: `Cooldown` (seconds between blasts; one swing = one blast).

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
| 1 | 25 m/s |
| 2 | 45 m/s |
| 3 | 80 m/s |

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
| 1 | 10 m | 12 m/s | 10 m/s |
| 2 | 16 m | 22 m/s | 16 m/s |
| 3 | 25 m | 40 m/s | 26 m/s |

- ini: `Radius1-3`, `PushSpeed1-3`, `PullSpeed1-3`, `Cooldown`.

## Lightning Strike
*Mouse power.* A small blue crosshair appears in the middle of the screen. Point the camera at a spot and click **LMB**: a lightning bolt hits it.

| Level | Blast |
|---|---|
| 1 | Grenade |
| 2 | Rocket |
| 3 | Car bomb + fire |

- Pointing at the sky does nothing, because there's nothing to hit.
- Bullet Time doesn't slow down the bolt animation.
- ini: `Range`, `Cooldown`.

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

## Spawn Vehicle
One-shot: select it in the menu and press **Enter**. A vehicle appears 6 m in front of CJ. It doesn't work inside interiors.
- ini: `Model` (400-611, e.g. 411 Infernus, 520 Hydra, 432 Rhino, 425 Hunter).

---

## Fun combos
- **Flash:** Super Speed 3 + Fire Trail 2.
- **Superman:** Flight 2 + Invulnerability + Force (push from the air).
- **Max Payne:** Bullet Time 2 + Explosion Punch 1.
- **Storm god:** Flight 1 (hover high) + Lightning Strike 3.
