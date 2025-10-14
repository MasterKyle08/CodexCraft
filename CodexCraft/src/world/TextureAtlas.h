#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "BlockRegistry.h"

namespace CodexCraft::World {

struct AtlasRect {
    float uMin{ 0.0f };
    float vMin{ 0.0f };
    float uMax{ 0.0f };
    float vMax{ 0.0f };
};

struct AtlasUV {
    float u{ 0.0f };
    float v{ 0.0f };
};

class TextureAtlas {
public:
    static constexpr std::uint32_t kAtlasTextureSize = 1024;
    static constexpr std::uint32_t kAtlasTileSize = 64;
    static constexpr std::uint32_t kAtlasTilesPerAxis = kAtlasTextureSize / kAtlasTileSize;
    static constexpr std::uint32_t kAtlasTileCount = kAtlasTilesPerAxis * kAtlasTilesPerAxis;
    static constexpr float kAtlasTexelSize = 1.0f / static_cast<float>(kAtlasTextureSize);
    static constexpr std::size_t kBlockCount = static_cast<std::size_t>(BlockId::Leaves) + 1;

    static TextureAtlas& Instance();

    [[nodiscard]] const AtlasRect& GetBlockFaceRect(BlockId id, std::size_t faceIndex) const noexcept;
    [[nodiscard]] AtlasUV ComputeUV(const AtlasCell& cell,
                                    std::uint16_t tileWidth,
                                    std::uint16_t tileHeight,
                                    std::uint16_t offsetU,
                                    std::uint16_t offsetV) const noexcept;

private:
    TextureAtlas();

    [[nodiscard]] static AtlasRect ComputeRectFromCell(const AtlasCell& cell) noexcept;

    std::array<std::array<AtlasRect, 6>, kBlockCount> m_blockFaceRects{};
};

} // namespace CodexCraft::World

