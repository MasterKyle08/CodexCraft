#pragma once

#include "Config.hpp"

namespace world
{
inline std::uint8_t select_lod(int chunkDistance)
{
    const auto settings = config::lod();
    // LOD0 is used within the near radius, LOD1 for mid distance and LOD2 for the far horizon.
    // Distances are specified in chunk units to make tuning intuitive.
    if (chunkDistance <= settings.lod0)
        return 0;
    if (chunkDistance <= settings.lod1)
        return 1;
    return 2;
}

} // namespace world
