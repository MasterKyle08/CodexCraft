#include "TextureAtlas.h"

#include <algorithm>

namespace CodexCraft::World {

namespace {

[[nodiscard]] constexpr std::size_t ClampFaceIndex(std::size_t faceIndex) noexcept {
    return std::min<std::size_t>(faceIndex, 5);
}

[[nodiscard]] std::size_t BlockIndex(BlockId id) noexcept {
    return std::clamp<std::size_t>(static_cast<std::size_t>(id), 0, TextureAtlas::kBlockCount - 1);
}

} // namespace

TextureAtlas& TextureAtlas::Instance() {
    static TextureAtlas atlas;
    return atlas;
}

TextureAtlas::TextureAtlas() {
    for (std::size_t block = 0; block < kBlockCount; ++block) {
        const BlockId id = static_cast<BlockId>(block);
        const BlockDefinition& definition = GetBlockDefinition(id);
        for (std::size_t face = 0; face < 6; ++face) {
            const AtlasCell& cell = definition.atlasCells[face];
            m_blockFaceRects[block][face] = ComputeRectFromCell(cell);
        }
    }
}

const AtlasRect& TextureAtlas::GetBlockFaceRect(BlockId id, std::size_t faceIndex) const noexcept {
    const std::size_t clampedFace = ClampFaceIndex(faceIndex);
    return m_blockFaceRects[BlockIndex(id)][clampedFace];
}

AtlasUV TextureAtlas::ComputeUV(const AtlasCell& cell,
                                std::uint16_t tileWidth,
                                std::uint16_t tileHeight,
                                std::uint16_t offsetU,
                                std::uint16_t offsetV) const noexcept {
    const AtlasRect rect = ComputeRectFromCell(cell);
    const float spanU = rect.uMax - rect.uMin;
    const float spanV = rect.vMax - rect.vMin;

    const float width = static_cast<float>(tileWidth == 0 ? 1 : tileWidth);
    const float height = static_cast<float>(tileHeight == 0 ? 1 : tileHeight);

    const float fractionU = static_cast<float>(offsetU) / width;
    const float fractionV = static_cast<float>(offsetV) / height;

    AtlasUV uv{};
    uv.u = rect.uMin + spanU * fractionU;
    uv.v = rect.vMin + spanV * fractionV;
    return uv;
}

AtlasRect TextureAtlas::ComputeRectFromCell(const AtlasCell& cell) noexcept {
    const float tileSpan = 1.0f / static_cast<float>(kAtlasTilesPerAxis);
    const float halfTexel = 0.5f * kAtlasTexelSize;

    const std::uint32_t maxIndex = kAtlasTilesPerAxis - 1;
    const std::uint32_t clampedU = std::min<std::uint32_t>(cell.u, maxIndex);
    const std::uint32_t clampedV = std::min<std::uint32_t>(cell.v, maxIndex);

    const float uMin = static_cast<float>(clampedU) * tileSpan + halfTexel;
    const float vMin = static_cast<float>(clampedV) * tileSpan + halfTexel;
    const float uMax = static_cast<float>(clampedU + 1) * tileSpan - halfTexel;
    const float vMax = static_cast<float>(clampedV + 1) * tileSpan - halfTexel;

    return AtlasRect{ uMin, vMin, uMax, vMax };
}

} // namespace CodexCraft::World

