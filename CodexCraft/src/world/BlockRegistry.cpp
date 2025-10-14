#include "BlockRegistry.h"

#include <array>

namespace CodexCraft::World {

namespace {

constexpr std::array<BlockDefinition, 8> kDefinitions = [] {
    std::array<BlockDefinition, 8> definitions{};

    auto setAllFaces = [](BlockDefinition& def, std::uint16_t u, std::uint16_t v) {
        def.atlasCells.fill({ u, v });
    };

    // Air
    definitions[static_cast<std::size_t>(BlockId::Air)] = BlockDefinition{};

    // Stone
    {
        BlockDefinition def{};
        def.isOpaque = true;
        setAllFaces(def, 0, 0);
        definitions[static_cast<std::size_t>(BlockId::Stone)] = def;
    }

    // Dirt
    {
        BlockDefinition def{};
        def.isOpaque = true;
        setAllFaces(def, 1, 0);
        definitions[static_cast<std::size_t>(BlockId::Dirt)] = def;
    }

    // Grass - top, bottom, sides use different cells.
    {
        BlockDefinition def{};
        def.isOpaque = true;
        def.atlasCells = {
            AtlasCell{ 2, 0 }, // +X
            AtlasCell{ 2, 0 }, // -X
            AtlasCell{ 2, 1 }, // +Y (top)
            AtlasCell{ 1, 0 }, // -Y (bottom)
            AtlasCell{ 2, 0 }, // +Z
            AtlasCell{ 2, 0 }, // -Z
        };
        definitions[static_cast<std::size_t>(BlockId::Grass)] = def;
    }

    // Sand
    {
        BlockDefinition def{};
        def.isOpaque = true;
        setAllFaces(def, 3, 0);
        definitions[static_cast<std::size_t>(BlockId::Sand)] = def;
    }

    // Water - treated as transparent
    {
        BlockDefinition def{};
        def.isOpaque = false;
        def.isTransparent = true;
        setAllFaces(def, 0, 1);
        definitions[static_cast<std::size_t>(BlockId::Water)] = def;
    }

    // Wood
    {
        BlockDefinition def{};
        def.isOpaque = true;
        def.atlasCells = {
            AtlasCell{ 1, 1 },
            AtlasCell{ 1, 1 },
            AtlasCell{ 1, 2 },
            AtlasCell{ 1, 2 },
            AtlasCell{ 1, 1 },
            AtlasCell{ 1, 1 },
        };
        definitions[static_cast<std::size_t>(BlockId::Wood)] = def;
    }

    // Leaves - semi-transparent foliage
    {
        BlockDefinition def{};
        def.isOpaque = false;
        def.isTransparent = true;
        setAllFaces(def, 2, 2);
        definitions[static_cast<std::size_t>(BlockId::Leaves)] = def;
    }

    return definitions;
}();

constexpr BlockDefinition kFallbackDefinition{};

} // namespace

const BlockDefinition& GetBlockDefinition(BlockId id) noexcept {
    const std::size_t index = static_cast<std::size_t>(id);
    if (index >= kDefinitions.size()) {
        return kFallbackDefinition;
    }

    return kDefinitions[index];
}

bool IsRenderable(BlockId id) noexcept {
    if (id == BlockId::Air) {
        return false;
    }

    const auto& def = GetBlockDefinition(id);
    return def.isOpaque || def.isTransparent;
}

bool IsTransparent(BlockId id) noexcept {
    const auto& def = GetBlockDefinition(id);
    return def.isTransparent && id != BlockId::Air;
}

} // namespace CodexCraft::World

