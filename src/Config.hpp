#pragma once

#include <cstdint>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace config
{
struct NoiseSettings
{
    int32_t seed = 1337;
    float frequency = 0.0025f;
    float amplitude = 32.0f;
    int32_t octaves = 4;
    float lacunarity = 2.0f;
    float gain = 0.5f;
    float baseHeight = 64.0f;
    float seaLevel = 62.0f;
};

struct LODSettings
{
    // Distance thresholds in chunk units. <= level0 is full resolution.
    int lod0 = 4;
    int lod1 = 8;
};

struct StreamSettings
{
    int loadRadius = 10;
    int meshRadius = 9;
    int renderRadius = 8;
};

struct AtlasSettings
{
    int tilesX = 4;
    int tilesY = 4;
    float padding = 0.5f; // Half texel padding to avoid bleeding.
    int textureResolution = 1024;
};

struct AppSettings
{
    glm::ivec2 windowSize{1600, 900};
    bool startFullscreen = false;
};

NoiseSettings noise();
LODSettings lod();
StreamSettings streaming();
AtlasSettings atlas();
AppSettings app();

} // namespace config
