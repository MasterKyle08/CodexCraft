#pragma once

#include <array>
#include <cstdint>

#include "Block.h"

namespace CodexCraft::World {

struct AtlasCell {
    std::uint16_t u{ 0 };
    std::uint16_t v{ 0 };
};

struct BlockDefinition {
    bool isOpaque{ false };
    bool isTransparent{ false };
    std::array<AtlasCell, 6> atlasCells{};
};

[[nodiscard]] const BlockDefinition& GetBlockDefinition(BlockId id) noexcept;
[[nodiscard]] bool IsRenderable(BlockId id) noexcept;
[[nodiscard]] bool IsTransparent(BlockId id) noexcept;

} // namespace CodexCraft::World

