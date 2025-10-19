#include "Chunk.hpp"

namespace world
{
Chunk::Chunk(ChunkCoord coord) : m_coord(coord)
{
    for (auto& dirty : m_dirty)
    {
        dirty.store(true);
    }
}

BlockID Chunk::get(int x, int y, int z) const
{
    const int sectionIdx = section_index(y);
    const int localY = y % SectionSize;
    return m_sections[sectionIdx].get(x, localY, z);
}

void Chunk::set(int x, int y, int z, BlockID id)
{
    const int sectionIdx = section_index(y);
    const int localY = y % SectionSize;
    m_sections[sectionIdx].set(x, localY, z, id);
    for (auto& dirty : m_dirty)
    {
        dirty.store(true, std::memory_order_relaxed);
    }
}

bool Chunk::needs_remesh(std::uint8_t lod) const
{
    if (lod >= m_dirty.size())
        return false;
    return m_dirty[lod].load(std::memory_order_relaxed);
}

void Chunk::mark_dirty(std::uint8_t lod)
{
    if (lod < m_dirty.size())
    {
        m_dirty[lod].store(true, std::memory_order_relaxed);
    }
}

void Chunk::clear_dirty(std::uint8_t lod) const
{
    if (lod < m_dirty.size())
    {
        m_dirty[lod].store(false, std::memory_order_relaxed);
    }
}

glm::vec3 Chunk::world_position() const
{
    return {static_cast<float>(m_coord.x * ChunkWidth), 0.0f, static_cast<float>(m_coord.z * ChunkDepth)};
}

} // namespace world
