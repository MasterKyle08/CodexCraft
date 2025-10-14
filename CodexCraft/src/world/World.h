#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <shared_mutex>
#include <unordered_map>
#include <vector>

#include "Chunk.h"
#include "ChunkPosition.h"
#include "ThreadSafeQueue.h"
#include "meshing/ChunkMeshCache.h"

namespace CodexCraft::World {

struct ChunkTask {
    ChunkPosition position{};
    std::shared_ptr<Chunk> chunk{};
};

class World {
public:
    World();

    void SetViewDistance(std::int32_t viewDistance);
    [[nodiscard]] std::int32_t GetViewDistance() const noexcept { return m_viewDistance; }

    void Update(const std::array<float, 3>& cameraPosition);

    [[nodiscard]] std::vector<ChunkPosition> GetLoadedChunks() const;

    bool TryDequeueGenerationTask(ChunkTask& outTask);
    bool TryDequeueMeshTask(ChunkTask& outTask);
    ChunkTask WaitForGenerationTask();
    ChunkTask WaitForMeshTask();

    void QueueChunkForMeshing(const std::shared_ptr<Chunk>& chunk);

    [[nodiscard]] Meshing::ChunkMeshCache& MeshCache() noexcept { return m_meshCache; }
    [[nodiscard]] const Meshing::ChunkMeshCache& MeshCache() const noexcept { return m_meshCache; }

private:
    [[nodiscard]] ChunkPosition WorldToChunkPosition(float worldX, float worldZ) const noexcept;
    void EnsureChunksAround(const ChunkPosition& center);
    void UnloadDistantChunks(const ChunkPosition& center);

    std::int32_t m_viewDistance{ 2 };
    mutable std::shared_mutex m_chunkMutex;
    std::unordered_map<ChunkPosition, std::shared_ptr<Chunk>, ChunkPositionHasher> m_chunks;
    ThreadSafeQueue<ChunkTask> m_generationQueue;
    ThreadSafeQueue<ChunkTask> m_meshQueue;
    Meshing::ChunkMeshCache m_meshCache;
};

} // namespace CodexCraft::World

