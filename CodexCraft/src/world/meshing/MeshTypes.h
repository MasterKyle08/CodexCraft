#pragma once

#include <cstdint>
#include <vector>

#include <d3d11.h>
#include <wrl/client.h>

#include "../BlockRegistry.h"

namespace CodexCraft::World::Meshing {

struct ChunkVertex {
    float position[3];
    float normal[3];
    std::uint16_t atlasCellU;
    std::uint16_t atlasCellV;
    std::uint16_t tileWidth;
    std::uint16_t tileHeight;
    std::uint16_t cornerOffsetU;
    std::uint16_t cornerOffsetV;
};

struct MeshBuffers {
    Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
    std::uint32_t vertexStride{ sizeof(ChunkVertex) };
    std::uint32_t vertexCapacity{ 0 };
    std::uint32_t indexCapacity{ 0 };
    std::uint32_t indexCount{ 0 };
};

struct ChunkMeshData {
    std::vector<ChunkVertex> opaqueVertices;
    std::vector<std::uint32_t> opaqueIndices;
    std::vector<ChunkVertex> transparentVertices;
    std::vector<std::uint32_t> transparentIndices;
};

} // namespace CodexCraft::World::Meshing

