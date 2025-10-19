#pragma once

#include "Config.hpp"
#include "World/Block.hpp"

#include <glm/vec2.hpp>

namespace world
{
struct AtlasUV
{
    glm::vec2 uv0;
    glm::vec2 uv1;
};

// Converts an atlas tile index into normalized UV coordinates. Padding is applied in
// normalized texture space to sample the interior of each tile and avoid bleeding.
inline AtlasUV atlas_uv(const BlockFaceUV& tile)
{
    const auto atlas = config::atlas();
    const float tileWidth = 1.0f / static_cast<float>(atlas.tilesX);
    const float tileHeight = 1.0f / static_cast<float>(atlas.tilesY);
    const float paddingX = atlas.padding / static_cast<float>(atlas.textureResolution);
    const float paddingY = atlas.padding / static_cast<float>(atlas.textureResolution);

    const float u0 = tile.tileX * tileWidth + paddingX;
    const float v0 = tile.tileY * tileHeight + paddingY;
    const float u1 = (tile.tileX + 1) * tileWidth - paddingX;
    const float v1 = (tile.tileY + 1) * tileHeight - paddingY;

    return {glm::vec2{u0, v0}, glm::vec2{u1, v1}};
}

} // namespace world
