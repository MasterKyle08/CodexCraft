#pragma once

#include "MeshTypes.h"

#include "../Chunk.h"

namespace CodexCraft::World::Meshing {

class GreedyMesher {
public:
    GreedyMesher() = default;

    [[nodiscard]] ChunkMeshData BuildMesh(const Chunk& chunk) const;
};

} // namespace CodexCraft::World::Meshing

