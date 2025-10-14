#pragma once

#include <cstdint>

namespace CodexCraft::World {

// Common block identifiers used throughout the world simulation.
enum class BlockId : std::uint16_t {
    Air = 0,
    Stone,
    Dirt,
    Grass,
    Sand,
    Water,
    Wood,
    Leaves,
};

// Maximum value for both block light and skylight channels.
constexpr std::uint8_t kMaxLightLevel = 15;

// Lightweight container describing a block's metadata and lighting payloads.
struct BlockData {
    constexpr BlockData() noexcept = default;
    constexpr BlockData(BlockId idValue,
                        std::uint8_t metadataValue,
                        std::uint8_t blockLightValue,
                        std::uint8_t skyLightValue) noexcept
        : id(idValue)
        , metadata(metadataValue)
        , blockLight(blockLightValue)
        , skyLight(skyLightValue) {}

    BlockId id{ BlockId::Air };
    std::uint8_t metadata{ 0 };
    std::uint8_t blockLight{ 0 };
    std::uint8_t skyLight{ 0 };
};

} // namespace CodexCraft::World

