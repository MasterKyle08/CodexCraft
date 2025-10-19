#pragma once

#include <array>
#include <cstdint>
#include <string_view>

namespace world
{
using BlockID = std::uint16_t;

enum class BlockFace : std::uint8_t
{
    PosX,
    NegX,
    PosY,
    NegY,
    PosZ,
    NegZ,
    Count
};

enum BlockFlags : std::uint8_t
{
    None = 0,
    Opaque = 1 << 0,
    Transparent = 1 << 1,
    Fluid = 1 << 2
};

inline bool has_flag(std::uint8_t flags, BlockFlags flag)
{
    return (flags & static_cast<std::uint8_t>(flag)) != 0;
}

struct BlockFaceUV
{
    int tileX = 0;
    int tileY = 0;
};

struct BlockDefinition
{
    std::string_view name{};
    std::uint8_t flags = BlockFlags::Opaque;
    std::array<BlockFaceUV, static_cast<int>(BlockFace::Count)> faces{};
};

constexpr BlockID BlockAir = 0;

} // namespace world
