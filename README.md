# SUMMIT: The Lost Expedition — Nintendo 3DS prototype

A native Nintendo 3DS homebrew prototype built with devkitARM, libctru and citro3d.

## Current prototype
- Top screen: low-poly 3D mountain scene, first-person-style viewpoint and visible scout hands.
- Bottom touchscreen: expedition map, height, stamina, grip state and control guide.
- Controls: D-pad up climbs, down rests, left/right traverses; A/B toggle left/right grips; X pulls upward; Y rests; touchscreen left/right halves engage grips and upper area climbs.
- START exits to Homebrew Launcher.

## Build
This project requires a devkitPro environment with devkitARM, libctru and citro3d installed.

```sh
make
```

The output is `SUMMIT.3dsx` (and `SUMMIT.smdh`). The included GitHub Actions workflow builds it in the devkitPro devkitARM container and uploads the artifact.

## Development notes
This is an early vertical-slice prototype, not the complete game yet. It establishes the two-screen layout, low-poly rendering path and basic climb/stamina/grip loop. Next milestones: proper handhold detection, collision and ledge transitions, world chunking/biomes, inventory items, sound, save data and multiplayer investigation.
