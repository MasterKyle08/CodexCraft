#pragma once

#include "Chunk.hpp"

#include "Renderer/Mesh.hpp"

#include <vector>

namespace world
{
struct NeighborSet
{
    const Chunk* posX = nullptr;
    const Chunk* negX = nullptr;
    const Chunk* posZ = nullptr;
    const Chunk* negZ = nullptr;
};

class GreedyMesher
{
  public:
    static void build(const Chunk& chunk,
                      const NeighborSet& neighbors,
                      std::uint8_t lod,
                      bool opaquePass,
                      std::vector<renderer::ChunkVertex>& vertices,
                      std::vector<std::uint32_t>& indices);
};

} // namespace world
