#include "WorldGen.hpp"

#include "BlockRegistry.hpp"

#include <glm/vec3.hpp>

namespace world
{
WorldGenerator::WorldGenerator()
{
    set_config(WorldGenConfig{});
}

void WorldGenerator::set_config(const WorldGenConfig& config)
{
    m_config = config;
    m_noise.SetSeed(config.seed);
    m_noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    m_noise.SetFrequency(config.frequency);
    m_noise.SetFractalType(FastNoiseLite::FractalType_FBm);
    m_noise.SetFractalOctaves(config.octaves);
    m_noise.SetFractalLacunarity(config.lacunarity);
    m_noise.SetFractalGain(config.gain);
}

void WorldGenerator::generate_chunk(Chunk& chunk) const
{
    const glm::vec3 origin = chunk.world_position();

    // Height is computed via fractal noise; amplitude, frequency and octave controls are
    // exposed through WorldGenConfig so designers can easily tune the terrain profile.
    for (int x = 0; x < ChunkWidth; ++x)
    {
        for (int z = 0; z < ChunkDepth; ++z)
        {
            const float worldX = origin.x + static_cast<float>(x);
            const float worldZ = origin.z + static_cast<float>(z);
            const float height = noise_height(worldX, worldZ);
            const int surfaceY = static_cast<int>(height);

            for (int y = 0; y < ChunkHeight; ++y)
            {
                BlockID block = BlockAir;
                if (y <= surfaceY)
                {
                    if (y == surfaceY)
                    {
                        block = surface_block(height, static_cast<float>(y));
                    }
                    else if (y > surfaceY - 4)
                    {
                        block = 2; // dirt
                    }
                    else
                    {
                        block = 3; // stone
                    }
                }
                else if (static_cast<float>(y) < m_config.seaLevel)
                {
                    block = 4; // water
                }

                chunk.set(x, y, z, block);
            }
        }
    }
}

float WorldGenerator::noise_height(float x, float z) const
{
    float amplitude = m_config.amplitude;
    float value = 0.0f;
    float frequency = m_config.frequency;
    float weight = 1.0f;
    for (int i = 0; i < m_config.octaves; ++i)
    {
        value += m_noise.GetNoise(x * frequency, z * frequency) * amplitude * weight;
        weight *= m_config.gain;
        frequency *= m_config.lacunarity;
    }
    return m_config.baseHeight + value;
}

BlockID WorldGenerator::surface_block(float height, float y) const
{
    const float normalized = height - m_config.baseHeight;
    if (y < m_config.seaLevel - 2.0f)
    {
        return 5; // sand
    }
    if (normalized > 40.0f)
    {
        return 6; // snow
    }
    return 1; // grass
}

} // namespace world
