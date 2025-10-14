#pragma once

#include <memory>
#include <unordered_map>

#include "../Chunk.h"
#include "../ChunkPosition.h"
#include "GreedyMesher.h"
#include "MeshTypes.h"

namespace CodexCraft::World::Meshing {

struct ChunkMesh {
    MeshBuffers opaque;
    MeshBuffers transparent;
    std::uint64_t revision{ 0 };
};

class ChunkMeshCache {
public:
    ChunkMeshCache();

    void UpdateChunkMesh(const std::shared_ptr<Chunk>& chunk);
    void RemoveChunk(const ChunkPosition& position);
    void Clear();

    [[nodiscard]] const ChunkMesh* GetChunkMesh(const ChunkPosition& position) const noexcept;

private:
    void UploadSection(MeshBuffers& buffers,
                       const std::vector<ChunkVertex>& vertices,
                       const std::vector<std::uint32_t>& indices);

    GreedyMesher m_mesher;
    std::unordered_map<ChunkPosition, ChunkMesh, ChunkPositionHasher> m_cache;
};

} // namespace CodexCraft::World::Meshing

