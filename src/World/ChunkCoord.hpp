#pragma once

#include "Util/Hash.hpp"

#include <cmath>
#include <cstdint>
#include <glm/vec3.hpp>
#include <tuple>

namespace world
{
struct ChunkCoord
{
    int x = 0;
    int z = 0;

    auto operator<=>(const ChunkCoord&) const = default;
};

inline ChunkCoord from_world(const glm::vec3& position)
{
    constexpr int chunkSize = 16;
    return {
        static_cast<int>(std::floor(position.x / chunkSize)),
        static_cast<int>(std::floor(position.z / chunkSize))};
}

} // namespace world

namespace std
{
template <> struct hash<world::ChunkCoord>
{
    std::size_t operator()(const world::ChunkCoord& coord) const noexcept
    {
        std::size_t seed = 0;
        seed = util::hash_combine(seed, static_cast<std::size_t>(coord.x));
        seed = util::hash_combine(seed, static_cast<std::size_t>(coord.z));
        return seed;
    }
};
} // namespace std
