#pragma once

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>

#include "Block.h"

namespace CodexCraft::World {

constexpr std::int32_t kChunkWidth = 32;
constexpr std::int32_t kChunkDepth = 32;
constexpr std::int32_t kChunkHeight = 256;
constexpr std::size_t kChunkVolume = static_cast<std::size_t>(kChunkWidth) *
                                      static_cast<std::size_t>(kChunkDepth) *
                                      static_cast<std::size_t>(kChunkHeight);

class Chunk {
public:
    Chunk(std::int32_t chunkX, std::int32_t chunkZ) noexcept;

    [[nodiscard]] std::int32_t GetChunkX() const noexcept { return m_chunkX; }
    [[nodiscard]] std::int32_t GetChunkZ() const noexcept { return m_chunkZ; }

    [[nodiscard]] BlockId GetBlockId(std::int32_t x, std::int32_t y, std::int32_t z) const noexcept;
    void SetBlockId(std::int32_t x, std::int32_t y, std::int32_t z, BlockId id) noexcept;

    [[nodiscard]] std::uint8_t GetMetadata(std::int32_t x, std::int32_t y, std::int32_t z) const noexcept;
    void SetMetadata(std::int32_t x, std::int32_t y, std::int32_t z, std::uint8_t value) noexcept;

    [[nodiscard]] std::uint8_t GetBlockLight(std::int32_t x, std::int32_t y, std::int32_t z) const noexcept;
    void SetBlockLight(std::int32_t x, std::int32_t y, std::int32_t z, std::uint8_t value) noexcept;

    [[nodiscard]] std::uint8_t GetSkyLight(std::int32_t x, std::int32_t y, std::int32_t z) const noexcept;
    void SetSkyLight(std::int32_t x, std::int32_t y, std::int32_t z, std::uint8_t value) noexcept;

    [[nodiscard]] const std::array<BlockId, kChunkVolume>& Blocks() const noexcept { return m_blocks; }
    [[nodiscard]] const std::array<std::uint8_t, kChunkVolume>& Metadata() const noexcept { return m_metadata; }
    [[nodiscard]] const std::array<std::uint8_t, kChunkVolume>& BlockLight() const noexcept { return m_blockLight; }
    [[nodiscard]] const std::array<std::uint8_t, kChunkVolume>& SkyLight() const noexcept { return m_skyLight; }

private:
    [[nodiscard]] static constexpr std::size_t Index(std::int32_t x, std::int32_t y, std::int32_t z) noexcept {
        return (static_cast<std::size_t>(y) * static_cast<std::size_t>(kChunkDepth) + static_cast<std::size_t>(z)) *
                   static_cast<std::size_t>(kChunkWidth) +
               static_cast<std::size_t>(x);
    }

    static void AssertInRange(std::int32_t x, std::int32_t y, std::int32_t z) noexcept {
        assert(x >= 0 && x < kChunkWidth);
        assert(z >= 0 && z < kChunkDepth);
        assert(y >= 0 && y < kChunkHeight);
    }

    std::array<BlockId, kChunkVolume> m_blocks{};
    std::array<std::uint8_t, kChunkVolume> m_metadata{};
    std::array<std::uint8_t, kChunkVolume> m_blockLight{};
    std::array<std::uint8_t, kChunkVolume> m_skyLight{};
    std::int32_t m_chunkX{ 0 };
    std::int32_t m_chunkZ{ 0 };
};

inline Chunk::Chunk(std::int32_t chunkX, std::int32_t chunkZ) noexcept
    : m_chunkX(chunkX)
    , m_chunkZ(chunkZ) {}

inline BlockId Chunk::GetBlockId(std::int32_t x, std::int32_t y, std::int32_t z) const noexcept {
    AssertInRange(x, y, z);
    return m_blocks[Index(x, y, z)];
}

inline void Chunk::SetBlockId(std::int32_t x, std::int32_t y, std::int32_t z, BlockId id) noexcept {
    AssertInRange(x, y, z);
    m_blocks[Index(x, y, z)] = id;
}

inline std::uint8_t Chunk::GetMetadata(std::int32_t x, std::int32_t y, std::int32_t z) const noexcept {
    AssertInRange(x, y, z);
    return m_metadata[Index(x, y, z)];
}

inline void Chunk::SetMetadata(std::int32_t x, std::int32_t y, std::int32_t z, std::uint8_t value) noexcept {
    AssertInRange(x, y, z);
    m_metadata[Index(x, y, z)] = value;
}

inline std::uint8_t Chunk::GetBlockLight(std::int32_t x, std::int32_t y, std::int32_t z) const noexcept {
    AssertInRange(x, y, z);
    return m_blockLight[Index(x, y, z)];
}

inline void Chunk::SetBlockLight(std::int32_t x, std::int32_t y, std::int32_t z, std::uint8_t value) noexcept {
    AssertInRange(x, y, z);
    m_blockLight[Index(x, y, z)] = value;
}

inline std::uint8_t Chunk::GetSkyLight(std::int32_t x, std::int32_t y, std::int32_t z) const noexcept {
    AssertInRange(x, y, z);
    return m_skyLight[Index(x, y, z)];
}

inline void Chunk::SetSkyLight(std::int32_t x, std::int32_t y, std::int32_t z, std::uint8_t value) noexcept {
    AssertInRange(x, y, z);
    m_skyLight[Index(x, y, z)] = value;
}

} // namespace CodexCraft::World

