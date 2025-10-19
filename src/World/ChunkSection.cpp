#include "ChunkSection.hpp"

#include <algorithm>
#include <cassert>

namespace world
{
ChunkSection::ChunkSection()
{
    m_blocks.fill(BlockAir);
}

BlockID ChunkSection::get(int x, int y, int z) const
{
    assert(x >= 0 && x < SectionSize);
    assert(y >= 0 && y < SectionSize);
    assert(z >= 0 && z < SectionSize);
    return m_blocks[index(x, y, z)];
}

void ChunkSection::set(int x, int y, int z, BlockID id)
{
    assert(x >= 0 && x < SectionSize);
    assert(y >= 0 && y < SectionSize);
    assert(z >= 0 && z < SectionSize);
    m_blocks[index(x, y, z)] = id;
    if (id != BlockAir)
    {
        m_isEmpty = false;
    }
}

int ChunkSection::index(int x, int y, int z)
{
    return x + SectionSize * (z + SectionSize * y);
}

} // namespace world
