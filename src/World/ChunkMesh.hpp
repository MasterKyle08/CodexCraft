#pragma once

#include "Chunk.hpp"
#include "Renderer/Mesh.hpp"

#include <array>
#include <vector>

namespace world
{
struct MeshBuffers
{
    std::vector<renderer::ChunkVertex> vertices;
    std::vector<std::uint32_t> indices;
};

struct LodMesh
{
    renderer::Mesh opaque;
    renderer::Mesh transparent;
    bool hasOpaque = false;
    bool hasTransparent = false;
};

class ChunkMesh
{
  public:
    ChunkMesh();

    MeshBuffers& cpu_opaque(std::uint8_t lod) { return m_cpuOpaque[lod]; }
    MeshBuffers& cpu_transparent(std::uint8_t lod) { return m_cpuTransparent[lod]; }

    void upload(std::uint8_t lod);
    void draw_opaque(std::uint8_t lod) const;
    void draw_transparent(std::uint8_t lod) const;

  private:
    std::array<MeshBuffers, 3> m_cpuOpaque;
    std::array<MeshBuffers, 3> m_cpuTransparent;
    std::array<LodMesh, 3> m_gpuMeshes;
};

} // namespace world
