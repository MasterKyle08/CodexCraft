#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <shared_mutex>
#include <unordered_map>
#include <vector>

#include "Chunk.h"
#include "ThreadSafeQueue.h"

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

private:
    [[nodiscard]] ChunkPosition WorldToChunkPosition(float worldX, float worldZ) const noexcept;
    void EnsureChunksAround(const ChunkPosition& center);
    void UnloadDistantChunks(const ChunkPosition& center);

    std::int32_t m_viewDistance{ 2 };
    mutable std::shared_mutex m_chunkMutex;
    std::unordered_map<ChunkPosition, std::shared_ptr<Chunk>, ChunkPositionHasher> m_chunks;
    ThreadSafeQueue<ChunkTask> m_generationQueue;
    ThreadSafeQueue<ChunkTask> m_meshQueue;
};

} // namespace CodexCraft::World

