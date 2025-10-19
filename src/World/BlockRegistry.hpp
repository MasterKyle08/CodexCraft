#pragma once

#include "Block.hpp"

#include <vector>

namespace world
{
class BlockRegistry
{
  public:
    BlockRegistry();

    const BlockDefinition& definition(BlockID id) const;
    std::uint8_t flags(BlockID id) const;

  private:
    std::vector<BlockDefinition> m_blocks;
};

const BlockRegistry& registry();

} // namespace world
