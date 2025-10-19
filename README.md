# CodexCraft Voxel Engine Starter

This project is a modern C++20/OpenGL starter for a Minecraft-like voxel engine. It focuses on chunked terrain streaming, CPU-side greedy meshing, distance-based LOD, and deterministic heightmap terrain generation.

## Building

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

## Running

Copy your 1024x1024 texture atlas to `assets/atlas.png` before running. Then launch the executable from the build directory:

```bash
./CodexCraft
```

Controls:
- **WASD**: Move horizontally
- **Space / Left Ctrl**: Move up / down
- **Mouse**: Look around (cursor locked)
- **ESC**: Quit
- **F1**: Toggle wireframe
- **F2**: Reload shaders
- **F3**: Print debug info (FPS, chunk stats)

## Notes

- `Config.hpp` exposes key tunables such as chunk radii, LOD distances, and noise parameters.
- `World/WorldGen.hpp` documents how the deterministic noise-based terrain is generated.
- `World/LOD.hpp` describes the distance thresholds used for coarse meshes.
- `World/AtlasUV.hpp` explains how UVs are derived from the atlas with padding.
- Shader sources reside in `shaders/` and are reloaded on F2.

Enjoy exploring and extending the engine!
