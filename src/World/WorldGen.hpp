#pragma once

#include "Chunk.hpp"
#include "Config.hpp"

#include <FastNoiseLite.h>

namespace world
{
struct WorldGenConfig
{
    int seed = config::noise().seed;
    float frequency = config::noise().frequency;
    float amplitude = config::noise().amplitude;
    int octaves = config::noise().octaves;
    float lacunarity = config::noise().lacunarity;
    float gain = config::noise().gain;
    float baseHeight = config::noise().baseHeight;
    float seaLevel = config::noise().seaLevel;
};

class WorldGenerator
{
  public:
    WorldGenerator();

    void set_config(const WorldGenConfig& config);
    void generate_chunk(Chunk& chunk) const;

  private:
    float noise_height(float x, float z) const;
    BlockID surface_block(float height, float y) const;

    WorldGenConfig m_config;
    mutable FastNoiseLite m_noise;
};

} // namespace world
