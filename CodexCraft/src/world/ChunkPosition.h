#pragma once

#include <cstdint>

namespace CodexCraft::World {

struct ChunkPosition {
    std::int32_t x{ 0 };
    std::int32_t z{ 0 };
};

inline bool operator==(const ChunkPosition& lhs, const ChunkPosition& rhs) noexcept {
    return lhs.x == rhs.x && lhs.z == rhs.z;
}

struct ChunkPositionHasher {
    [[nodiscard]] std::size_t operator()(const ChunkPosition& position) const noexcept {
        const std::uint64_t packedX = static_cast<std::uint32_t>(position.x);
        const std::uint64_t packedZ = static_cast<std::uint32_t>(position.z);
        return static_cast<std::size_t>((packedX << 32u) ^ packedZ);
    }
};

} // namespace CodexCraft::World

