#include "WorldStreamer.hpp"

#include "BlockRegistry.hpp"
#include "Util/Logging.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <queue>
#include <unordered_map>

#include <glm/vec3.hpp>

namespace world
{
namespace
{
std::vector<ChunkCoord> chunk_offsets(int radius)
{
    std::vector<ChunkCoord> coords;
    for (int dx = -radius; dx <= radius; ++dx)
    {
        for (int dz = -radius; dz <= radius; ++dz)
        {
            coords.push_back({dx, dz});
        }
    }
    std::sort(coords.begin(), coords.end(), [](const ChunkCoord& a, const ChunkCoord& b) {
        const int da = std::abs(a.x) + std::abs(a.z);
        const int db = std::abs(b.x) + std::abs(b.z);
        return da < db;
    });
    return coords;
}

const std::vector<ChunkCoord>& load_order(int radius)
{
    static std::unordered_map<int, std::vector<ChunkCoord>> cache;
    auto it = cache.find(radius);
    if (it != cache.end())
        return it->second;
    auto [emplaced, inserted] = cache.emplace(radius, chunk_offsets(radius));
    return emplaced->second;
}

} // namespace

WorldStreamer::WorldStreamer()
    : m_generationJobs(std::thread::hardware_concurrency())
    , m_meshingJobs(std::thread::hardware_concurrency())
{
}

void WorldStreamer::reload()
{
    std::unique_lock lock(m_chunkMutex);
    for (auto& [coord, entry] : m_chunks)
    {
        entry->chunk->mark_dirty(0);
        entry->chunk->mark_dirty(1);
        entry->chunk->mark_dirty(2);
        schedule_meshing(entry);
    }
}

std::shared_ptr<WorldStreamer::ChunkEntry> WorldStreamer::ensure_chunk(const ChunkCoord& coord)
{
    std::unique_lock lock(m_chunkMutex);
    auto it = m_chunks.find(coord);
    if (it != m_chunks.end())
    {
        return it->second;
    }

    auto entry = std::make_shared<ChunkEntry>();
    entry->chunk = std::make_shared<Chunk>(coord);
    m_chunks.emplace(coord, entry);
    entry->chunk->set_state(ChunkState::Generating);
    lock.unlock();

    schedule_generation(entry);
    return entry;
}

std::shared_ptr<WorldStreamer::ChunkEntry> WorldStreamer::find_entry(const ChunkCoord& coord) const
{
    std::shared_lock lock(m_chunkMutex);
    auto it = m_chunks.find(coord);
    if (it != m_chunks.end())
    {
        return it->second;
    }
    return nullptr;
}

void WorldStreamer::schedule_generation(const std::shared_ptr<ChunkEntry>& entry)
{
    auto weakEntry = std::weak_ptr<ChunkEntry>(entry);
    m_generationJobs.enqueue([this, weakEntry]() {
        if (auto strong = weakEntry.lock())
        {
            m_generator.generate_chunk(*strong->chunk);
            strong->chunk->set_state(ChunkState::MeshPending);
            strong->chunk->mark_dirty(0);
            strong->chunk->mark_dirty(1);
            strong->chunk->mark_dirty(2);

            const ChunkCoord coord = strong->chunk->coord();
            const std::array<ChunkCoord, 4> neighborOffsets = {{{1, 0}, {-1, 0}, {0, 1}, {0, -1}}};
            for (const auto& offset : neighborOffsets)
            {
                if (auto neighbor = find_entry({coord.x + offset.x, coord.z + offset.z}))
                {
                    neighbor->chunk->mark_dirty(0);
                    neighbor->chunk->mark_dirty(1);
                    neighbor->chunk->mark_dirty(2);
                    schedule_meshing(neighbor);
                }
            }

            schedule_meshing(strong);
        }
    });
}

NeighborSet WorldStreamer::gather_neighbors(const ChunkCoord& coord) const
{
    NeighborSet neighbors;
    if (auto entry = find_entry({coord.x + 1, coord.z}))
        neighbors.posX = entry->chunk.get();
    if (auto entry = find_entry({coord.x - 1, coord.z}))
        neighbors.negX = entry->chunk.get();
    if (auto entry = find_entry({coord.x, coord.z + 1}))
        neighbors.posZ = entry->chunk.get();
    if (auto entry = find_entry({coord.x, coord.z - 1}))
        neighbors.negZ = entry->chunk.get();
    return neighbors;
}

void WorldStreamer::schedule_meshing(const std::shared_ptr<ChunkEntry>& entry)
{
    if (entry->meshInFlight.exchange(true))
    {
        return;
    }

    auto weakEntry = std::weak_ptr<ChunkEntry>(entry);
    m_meshingJobs.enqueue([this, weakEntry]() {
        if (auto strong = weakEntry.lock())
        {
            if (strong->chunk->state() == ChunkState::Generating)
            {
                strong->meshInFlight = false;
                return;
            }

            MeshUpload upload;
            upload.coord = strong->chunk->coord();

            const NeighborSet neighbors = gather_neighbors(upload.coord);

            for (std::uint8_t lod = 0; lod < 3; ++lod)
            {
                auto& opaque = upload.opaque[lod];
                auto& transparent = upload.transparent[lod];
                GreedyMesher::build(*strong->chunk, neighbors, lod, true, opaque.vertices, opaque.indices);
                GreedyMesher::build(*strong->chunk, neighbors, lod, false, transparent.vertices, transparent.indices);
            }

            {
                std::lock_guard lock(m_uploadMutex);
                m_pendingUploads.push_back(std::move(upload));
            }
        }
    });
}

void WorldStreamer::process_uploads()
{
    std::vector<MeshUpload> uploads;
    {
        std::lock_guard lock(m_uploadMutex);
        uploads.swap(m_pendingUploads);
    }

    if (uploads.empty())
        return;

    for (auto& upload : uploads)
    {
        auto entry = find_entry(upload.coord);
        if (!entry)
            continue;

        for (std::uint8_t lod = 0; lod < 3; ++lod)
        {
            entry->mesh.cpu_opaque(lod) = std::move(upload.opaque[lod]);
            entry->mesh.cpu_transparent(lod) = std::move(upload.transparent[lod]);
            entry->mesh.upload(lod);
            entry->chunk->clear_dirty(lod);
        }

        entry->chunk->set_state(ChunkState::Uploaded);
        entry->meshInFlight = false;
    }
}

void WorldStreamer::unload_far_chunks(const glm::vec3& cameraPosition)
{
    const auto settings = config::streaming();
    const int unloadRadius = settings.loadRadius + 2;
    const ChunkCoord cameraChunk = from_world(cameraPosition);

    std::vector<ChunkCoord> toRemove;
    {
        std::shared_lock lock(m_chunkMutex);
        for (const auto& [coord, entry] : m_chunks)
        {
            const int dx = coord.x - cameraChunk.x;
            const int dz = coord.z - cameraChunk.z;
            if (std::abs(dx) > unloadRadius || std::abs(dz) > unloadRadius)
            {
                if (!entry->meshInFlight.load())
                {
                    toRemove.push_back(coord);
                }
            }
        }
    }

    if (!toRemove.empty())
    {
        std::unique_lock lock(m_chunkMutex);
        for (const auto& coord : toRemove)
        {
            m_chunks.erase(coord);
        }
    }
}

void WorldStreamer::update(const glm::vec3& cameraPosition)
{
    const auto settings = config::streaming();
    const ChunkCoord cameraChunk = from_world(cameraPosition);

    // Maintain a toroidal set of chunks around the camera: load radius controls generation,
    // while additional passes below handle mesh uploads and far chunk eviction.
    const auto& offsets = load_order(settings.loadRadius);
    for (const auto& offset : offsets)
    {
        ChunkCoord coord{cameraChunk.x + offset.x, cameraChunk.z + offset.z};
        auto entry = ensure_chunk(coord);
        if (entry->chunk->state() == ChunkState::MeshPending || entry->chunk->needs_remesh(0) || entry->chunk->needs_remesh(1) ||
            entry->chunk->needs_remesh(2))
        {
            schedule_meshing(entry);
        }
    }

    process_uploads();
    unload_far_chunks(cameraPosition);
}

void WorldStreamer::gather_draw_commands(const renderer::Camera& camera,
                                         const renderer::Frustum& frustum,
                                         std::vector<DrawCommand>& opaque,
                                         std::vector<DrawCommand>& transparent)
{
    opaque.clear();
    transparent.clear();

    const glm::vec3 cameraPos = camera.position();
    const auto settings = config::streaming();
    const int renderRadius = settings.renderRadius;
    const ChunkCoord center = from_world(cameraPos);

    std::shared_lock lock(m_chunkMutex);
    for (const auto& [coord, entry] : m_chunks)
    {
        const int dx = coord.x - center.x;
        const int dz = coord.z - center.z;
        if (std::abs(dx) > renderRadius || std::abs(dz) > renderRadius)
            continue;

        if (entry->chunk->state() != ChunkState::Uploaded)
            continue;

        const glm::vec3 position = entry->chunk->world_position();
        const glm::vec3 min = position;
        const glm::vec3 max = position + glm::vec3(ChunkWidth, static_cast<float>(ChunkHeight), ChunkDepth);
        if (!frustum.intersects(min, max))
            continue;

        const int manhattan = std::max(std::abs(dx), std::abs(dz));
        const std::uint8_t lod = select_lod(manhattan);

        opaque.push_back(DrawCommand{entry->chunk.get(), &entry->mesh, lod});
        transparent.push_back(DrawCommand{entry->chunk.get(), &entry->mesh, lod});
    }
}

} // namespace world

StreamerStats WorldStreamer::stats() const
{
    StreamerStats stats;
    std::shared_lock lock(m_chunkMutex);
    stats.totalChunks = m_chunks.size();
    for (const auto& [coord, entry] : m_chunks)
    {
        switch (entry->chunk->state())
        {
        case ChunkState::Unloaded:
            break;
        case ChunkState::Generating:
            ++stats.generating;
            break;
        case ChunkState::MeshPending:
            ++stats.meshPending;
            break;
        case ChunkState::MeshReady:
            ++stats.meshPending;
            break;
        case ChunkState::Uploaded:
            ++stats.uploaded;
            break;
        case ChunkState::Visible:
            ++stats.uploaded;
            break;
        }
        if (entry->meshInFlight.load())
        {
            ++stats.meshing;
        }
    }
    {
        std::lock_guard lockUploads(m_uploadMutex);
        stats.pendingUploads = m_pendingUploads.size();
    }
    return stats;
}

} // namespace world
