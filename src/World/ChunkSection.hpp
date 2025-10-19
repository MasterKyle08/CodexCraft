#pragma once

#include "Block.hpp"

#include <array>
#include <cstdint>

namespace world
{
constexpr int SectionSize = 16;
constexpr int SectionVolume = SectionSize * SectionSize * SectionSize;

class ChunkSection
{
  public:
    ChunkSection();

    BlockID get(int x, int y, int z) const;
    void set(int x, int y, int z, BlockID id);

    bool empty() const { return m_isEmpty; }

  private:
    static int index(int x, int y, int z);

    std::array<BlockID, SectionVolume> m_blocks{};
    bool m_isEmpty = true;
};

} // namespace world
