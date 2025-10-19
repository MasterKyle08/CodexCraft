#pragma once

#include "Chunk.hpp"
#include "ChunkMesh.hpp"
#include "GreedyMesher.hpp"
#include "LOD.hpp"
#include "WorldGen.hpp"

#include "Config.hpp"
#include "Core/JobSystem.hpp"
#include "Renderer/Camera.hpp"
#include "Renderer/Frustum.hpp"

#include <array>
#include <atomic>
#include <functional>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <unordered_map>
#include <vector>

namespace world
{
struct DrawCommand
{
    Chunk* chunk = nullptr;
    ChunkMesh* mesh = nullptr;
    std::uint8_t lod = 0;
};

struct StreamerStats
{
    std::size_t totalChunks = 0;
    std::size_t generating = 0;
    std::size_t meshPending = 0;
    std::size_t uploaded = 0;
    std::size_t meshing = 0;
    std::size_t pendingUploads = 0;
};

class WorldStreamer
{
  public:
    WorldStreamer();

    void update(const glm::vec3& cameraPosition);
    void gather_draw_commands(const renderer::Camera& camera,
                              const renderer::Frustum& frustum,
                              std::vector<DrawCommand>& opaque,
                              std::vector<DrawCommand>& transparent);

    void reload();

    StreamerStats stats() const;
    std::size_t pending_generation_jobs() const { return m_generationJobs.pending_jobs(); }
    std::size_t pending_meshing_jobs() const { return m_meshingJobs.pending_jobs(); }

  private:
    struct ChunkEntry
    {
        ChunkPtr chunk;
        ChunkMesh mesh;
        std::atomic_bool meshInFlight{false};
    };

    struct MeshUpload
    {
        ChunkCoord coord;
        std::array<MeshBuffers, 3> opaque;
        std::array<MeshBuffers, 3> transparent;
    };

    std::shared_ptr<ChunkEntry> ensure_chunk(const ChunkCoord& coord);
    std::shared_ptr<ChunkEntry> find_entry(const ChunkCoord& coord) const;
    void schedule_generation(const std::shared_ptr<ChunkEntry>& entry);
    void schedule_meshing(const std::shared_ptr<ChunkEntry>& entry);
    NeighborSet gather_neighbors(const ChunkCoord& coord) const;
    void process_uploads();
    void unload_far_chunks(const glm::vec3& cameraPosition);

    core::JobSystem m_generationJobs;
    core::JobSystem m_meshingJobs;
    WorldGenerator m_generator;

    mutable std::shared_mutex m_chunkMutex;
    std::unordered_map<ChunkCoord, std::shared_ptr<ChunkEntry>> m_chunks;

    std::mutex m_uploadMutex;
    std::vector<MeshUpload> m_pendingUploads;
};

} // namespace world
