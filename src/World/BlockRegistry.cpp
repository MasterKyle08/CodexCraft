#include "BlockRegistry.hpp"

#include <cassert>

namespace world
{
namespace
{
BlockDefinition make_block(std::string_view name, std::uint8_t flags, const BlockFaceUV& uv)
{
    BlockDefinition def;
    def.name = name;
    def.flags = flags;
    def.faces.fill(uv);
    return def;
}

BlockDefinition make_block(std::string_view name, std::uint8_t flags, const BlockFaceUV& top, const BlockFaceUV& side, const BlockFaceUV& bottom)
{
    BlockDefinition def;
    def.name = name;
    def.flags = flags;
    def.faces[static_cast<int>(BlockFace::PosY)] = top;
    def.faces[static_cast<int>(BlockFace::NegY)] = bottom;
    def.faces[static_cast<int>(BlockFace::PosX)] = side;
    def.faces[static_cast<int>(BlockFace::NegX)] = side;
    def.faces[static_cast<int>(BlockFace::PosZ)] = side;
    def.faces[static_cast<int>(BlockFace::NegZ)] = side;
    return def;
}
} // namespace

BlockRegistry::BlockRegistry()
{
    m_blocks.reserve(16);

    // Air placeholder.
    BlockDefinition air;
    air.name = "air";
    air.flags = BlockFlags::Transparent;
    m_blocks.push_back(air);

    m_blocks.push_back(make_block("grass", BlockFlags::Opaque, BlockFaceUV{0, 0}, BlockFaceUV{1, 0}, BlockFaceUV{2, 0}));
    m_blocks.push_back(make_block("dirt", BlockFlags::Opaque, BlockFaceUV{2, 0}));
    m_blocks.push_back(make_block("stone", BlockFlags::Opaque, BlockFaceUV{3, 0}));
    m_blocks.push_back(make_block("water", BlockFlags::Transparent | BlockFlags::Fluid, BlockFaceUV{0, 1}));
    m_blocks.push_back(make_block("sand", BlockFlags::Opaque, BlockFaceUV{1, 1}));
    m_blocks.push_back(make_block("snow", BlockFlags::Opaque, BlockFaceUV{2, 1}));
}

const BlockDefinition& BlockRegistry::definition(BlockID id) const
{
    assert(id < m_blocks.size());
    return m_blocks[id];
}

std::uint8_t BlockRegistry::flags(BlockID id) const
{
    assert(id < m_blocks.size());
    return m_blocks[id].flags;
}

const BlockRegistry& registry()
{
    static BlockRegistry g_registry;
    return g_registry;
}

} // namespace world
