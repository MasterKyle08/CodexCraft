#pragma once

#include "Block.hpp"
#include "ChunkCoord.hpp"
#include "ChunkSection.hpp"

#include <array>
#include <atomic>
#include <memory>

#include <glm/vec3.hpp>

namespace world
{
constexpr int ChunkWidth = 16;
constexpr int ChunkHeight = 256;
constexpr int ChunkDepth = 16;
constexpr int SectionCount = ChunkHeight / SectionSize;

enum class ChunkState : std::uint8_t
{
    Unloaded,
    Generating,
    MeshPending,
    MeshReady,
    Uploaded,
    Visible
};

// Represents a single column of 16x256x16 blocks, internally chunked into 16x16x16 sections.
// The state machine progresses from Unloaded -> Generating -> MeshPending -> Uploaded.
// Uploaded chunks are ready for rendering; once rendered they may be marked Visible.
class Chunk
{
  public:
    Chunk() = default;
    explicit Chunk(ChunkCoord coord);

    BlockID get(int x, int y, int z) const;
    void set(int x, int y, int z, BlockID id);

    ChunkSection& section(int index) { return m_sections[index]; }
    const ChunkSection& section(int index) const { return m_sections[index]; }

    ChunkCoord coord() const { return m_coord; }

    ChunkState state() const { return m_state.load(std::memory_order_relaxed); }
    void set_state(ChunkState state) { m_state.store(state, std::memory_order_relaxed); }

    bool needs_remesh(std::uint8_t lod) const;
    void mark_dirty(std::uint8_t lod);
    void clear_dirty(std::uint8_t lod) const;

    glm::vec3 world_position() const;

  private:
    static int section_index(int y) { return y / SectionSize; }

    ChunkCoord m_coord{};
    std::array<ChunkSection, SectionCount> m_sections{};
    mutable std::array<std::atomic_bool, 3> m_dirty{};
    std::atomic<ChunkState> m_state{ChunkState::Unloaded};
};

using ChunkPtr = std::shared_ptr<Chunk>;

} // namespace world
