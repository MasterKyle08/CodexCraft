#include "World.h"

#include <algorithm>
#include <cmath>
#include <utility>

namespace CodexCraft::World {

namespace {
std::int32_t ClampViewDistance(std::int32_t value) noexcept {
    return std::max<std::int32_t>(1, value);
}

bool IsWithinViewDistance(const ChunkPosition& center, const ChunkPosition& candidate, std::int32_t viewDistance) noexcept {
    const std::int32_t dx = candidate.x - center.x;
    const std::int32_t dz = candidate.z - center.z;
    const std::int32_t chebyshevDistance = std::max<std::int32_t>(std::abs(dx), std::abs(dz));
    return chebyshevDistance <= viewDistance;
}
} // namespace

World::World() = default;

void World::SetViewDistance(std::int32_t viewDistance) {
    const std::int32_t clamped = ClampViewDistance(viewDistance);
    if (clamped == m_viewDistance) {
        return;
    }

    m_viewDistance = clamped;
}

void World::Update(const std::array<float, 3>& cameraPosition) {
    const ChunkPosition cameraChunk = WorldToChunkPosition(cameraPosition[0], cameraPosition[2]);
    EnsureChunksAround(cameraChunk);
    UnloadDistantChunks(cameraChunk);
}

std::vector<ChunkPosition> World::GetLoadedChunks() const {
    std::shared_lock lock(m_chunkMutex);
    std::vector<ChunkPosition> positions;
    positions.reserve(m_chunks.size());
    for (const auto& [position, chunk] : m_chunks) {
        (void)chunk;
        positions.push_back(position);
    }

    return positions;
}

bool World::TryDequeueGenerationTask(ChunkTask& outTask) {
    return m_generationQueue.TryPop(outTask);
}

bool World::TryDequeueMeshTask(ChunkTask& outTask) {
    return m_meshQueue.TryPop(outTask);
}

ChunkTask World::WaitForGenerationTask() {
    return m_generationQueue.WaitPop();
}

ChunkTask World::WaitForMeshTask() {
    return m_meshQueue.WaitPop();
}

void World::QueueChunkForMeshing(const std::shared_ptr<Chunk>& chunk) {
    if (!chunk) {
        return;
    }

    ChunkTask task{};
    task.position = { chunk->GetChunkX(), chunk->GetChunkZ() };
    task.chunk = chunk;
    m_meshQueue.Push(std::move(task));

    m_meshCache.UpdateChunkMesh(chunk);
}

ChunkPosition World::WorldToChunkPosition(float worldX, float worldZ) const noexcept {
    const double chunkSpaceX = static_cast<double>(worldX) / static_cast<double>(kChunkWidth);
    const double chunkSpaceZ = static_cast<double>(worldZ) / static_cast<double>(kChunkDepth);
    const std::int32_t chunkX = static_cast<std::int32_t>(std::floor(chunkSpaceX));
    const std::int32_t chunkZ = static_cast<std::int32_t>(std::floor(chunkSpaceZ));
    return { chunkX, chunkZ };
}

void World::EnsureChunksAround(const ChunkPosition& center) {
    std::vector<ChunkTask> newChunks;
    {
        std::unique_lock lock(m_chunkMutex);
        for (std::int32_t dx = -m_viewDistance; dx <= m_viewDistance; ++dx) {
            for (std::int32_t dz = -m_viewDistance; dz <= m_viewDistance; ++dz) {
                const ChunkPosition position{ center.x + dx, center.z + dz };
                if (m_chunks.find(position) != m_chunks.end()) {
                    continue;
                }

                auto chunk = std::make_shared<Chunk>(position.x, position.z);
                m_chunks.emplace(position, chunk);

                ChunkTask task{};
                task.position = position;
                task.chunk = std::move(chunk);
                newChunks.push_back(std::move(task));
            }
        }
    }

    for (auto& task : newChunks) {
        m_generationQueue.Push(std::move(task));
    }
}

void World::UnloadDistantChunks(const ChunkPosition& center) {
    std::vector<ChunkPosition> toRemove;
    {
        std::shared_lock lock(m_chunkMutex);
        toRemove.reserve(m_chunks.size());
        for (const auto& [position, chunk] : m_chunks) {
            (void)chunk;
            if (!IsWithinViewDistance(center, position, m_viewDistance)) {
                toRemove.push_back(position);
            }
        }
    }

    if (toRemove.empty()) {
        return;
    }

    std::unique_lock lock(m_chunkMutex);
    for (const ChunkPosition& position : toRemove) {
        auto it = m_chunks.find(position);
        if (it != m_chunks.end() && !IsWithinViewDistance(center, position, m_viewDistance)) {
            m_meshCache.RemoveChunk(position);
            m_chunks.erase(it);
        }
    }
}

} // namespace CodexCraft::World

