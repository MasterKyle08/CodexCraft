# Texture Atlas Overview

- **Atlas resolution:** 1024×1024
- **Cell size:** 64×64 pixels (16×16 grid)
- **File location:** `assets/textures/blocks.png`

> **Tip:** If your artwork ships as `atlas.png`, copy or rename it to
> `assets/textures/blocks.png` so the runtime can locate it without any
> additional configuration.

The atlas is treated as a uniform grid. Each block face references a cell via
its `(u, v)` index in `BlockRegistry.cpp`, and the runtime atlas helper computes
padded UV rectangles directly from those indices. No external scripts are
required—the engine will automatically pick up new cells as soon as they are
referenced by a block definition.

When authoring new textures:

1. Reserve an unused cell in the atlas grid and place the texture art inside it.
2. Update the associated `AtlasCell` entries in `BlockRegistry.cpp` so the block
   points at the new cell indices.
3. Rebuild the project. The UV rectangles are derived at load time, applying a
   half-texel gutter to avoid sampling bleeding.

## Texture Cells

| Texture      | Cell (u, v) | Pixel Range   | Notes                                   |
|--------------|-------------|---------------|-----------------------------------------|
| `stone`      | (0, 0)      | 0–64, 0–64    | Base stone used for underground layers. |
| `dirt`       | (1, 0)      | 64–128, 0–64  | Exposed dirt without grass cover.       |
| `grass_side` | (2, 0)      | 128–192, 0–64 | Grass block side wall.                  |
| `sand`       | (3, 0)      | 192–256, 0–64 | Loose beach sand.                       |
| `water`      | (0, 1)      | 0–64, 64–128  | Still water surface.                    |
| `wood_side`  | (1, 1)      | 64–128, 64–128| Tree log bark.                          |
| `grass_top`  | (2, 1)      | 128–192, 64–128| Grass block top surface.               |
| `wood_top`   | (1, 2)      | 64–128, 128–192| Tree log cross-section.                |
| `leaves`     | (2, 2)      | 128–192, 128–192| Leaf canopy.                           |

