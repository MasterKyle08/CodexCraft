#include "ChunkMesh.hpp"

namespace world
{
ChunkMesh::ChunkMesh() = default;

void ChunkMesh::upload(std::uint8_t lod)
{
    auto& cpuOpaque = m_cpuOpaque[lod];
    auto& cpuTransparent = m_cpuTransparent[lod];
    auto& gpu = m_gpuMeshes[lod];

    if (!cpuOpaque.vertices.empty() && !cpuOpaque.indices.empty())
    {
        gpu.opaque.upload(cpuOpaque.vertices, cpuOpaque.indices);
        gpu.hasOpaque = true;
    }
    else
    {
        gpu.hasOpaque = false;
    }

    if (!cpuTransparent.vertices.empty() && !cpuTransparent.indices.empty())
    {
        gpu.transparent.upload(cpuTransparent.vertices, cpuTransparent.indices, true);
        gpu.hasTransparent = true;
    }
    else
    {
        gpu.hasTransparent = false;
    }
}

void ChunkMesh::draw_opaque(std::uint8_t lod) const
{
    const auto& mesh = m_gpuMeshes[lod];
    if (mesh.hasOpaque)
    {
        mesh.opaque.draw();
    }
}

void ChunkMesh::draw_transparent(std::uint8_t lod) const
{
    const auto& mesh = m_gpuMeshes[lod];
    if (mesh.hasTransparent)
    {
        mesh.transparent.draw();
    }
}

} // namespace world
