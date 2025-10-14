#include "ChunkMeshCache.h"

#include <cstring>
#include <system_error>

#include "../../platform/windows/Application.h"

namespace CodexCraft::World::Meshing {

namespace {

void ThrowIfFailed(HRESULT hr, const char* message) {
    if (FAILED(hr)) {
        throw std::system_error(hr, std::system_category(), message);
    }
}

} // namespace

using CodexCraft::Platform::Windows::Renderer;

ChunkMeshCache::ChunkMeshCache() = default;

void ChunkMeshCache::UploadSection(MeshBuffers& buffers,
                                   const std::vector<ChunkVertex>& vertices,
                                   const std::vector<std::uint32_t>& indices) {
    auto& renderer = Renderer::Instance();
    ID3D11Device* device = renderer.GetDevice();
    ID3D11DeviceContext* context = renderer.GetContext();

    buffers.indexCount = static_cast<std::uint32_t>(indices.size());

    if (!device || !context) {
        buffers.vertexBuffer.Reset();
        buffers.indexBuffer.Reset();
        buffers.vertexCapacity = 0;
        buffers.indexCapacity = 0;
        buffers.indexCount = 0;
        return;
    }

    if (vertices.empty() || indices.empty()) {
        buffers.vertexBuffer.Reset();
        buffers.indexBuffer.Reset();
        buffers.vertexCapacity = 0;
        buffers.indexCapacity = 0;
        buffers.indexCount = 0;
        return;
    }

    const UINT vertexCount = static_cast<UINT>(vertices.size());
    const UINT vertexBufferSize = vertexCount * sizeof(ChunkVertex);

    if (!buffers.vertexBuffer || buffers.vertexCapacity < vertexCount) {
        D3D11_BUFFER_DESC desc{};
        desc.ByteWidth = vertexBufferSize;
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        D3D11_SUBRESOURCE_DATA initData{};
        initData.pSysMem = vertices.data();

        Microsoft::WRL::ComPtr<ID3D11Buffer> newBuffer;
        ThrowIfFailed(device->CreateBuffer(&desc, &initData, &newBuffer),
                      "Failed to create chunk vertex buffer");
        buffers.vertexBuffer = std::move(newBuffer);
        buffers.vertexCapacity = vertexCount;
    } else {
        D3D11_MAPPED_SUBRESOURCE mapped{};
        ThrowIfFailed(context->Map(buffers.vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped),
                      "Failed to map chunk vertex buffer");
        std::memcpy(mapped.pData, vertices.data(), vertexBufferSize);
        context->Unmap(buffers.vertexBuffer.Get(), 0);
    }

    const UINT indexCount = static_cast<UINT>(indices.size());
    const UINT indexBufferSize = indexCount * sizeof(std::uint32_t);

    if (!buffers.indexBuffer || buffers.indexCapacity < indexCount) {
        D3D11_BUFFER_DESC desc{};
        desc.ByteWidth = indexBufferSize;
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        D3D11_SUBRESOURCE_DATA initData{};
        initData.pSysMem = indices.data();

        Microsoft::WRL::ComPtr<ID3D11Buffer> newBuffer;
        ThrowIfFailed(device->CreateBuffer(&desc, &initData, &newBuffer),
                      "Failed to create chunk index buffer");
        buffers.indexBuffer = std::move(newBuffer);
        buffers.indexCapacity = indexCount;
    } else {
        D3D11_MAPPED_SUBRESOURCE mapped{};
        ThrowIfFailed(context->Map(buffers.indexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped),
                      "Failed to map chunk index buffer");
        std::memcpy(mapped.pData, indices.data(), indexBufferSize);
        context->Unmap(buffers.indexBuffer.Get(), 0);
    }
}

void ChunkMeshCache::UpdateChunkMesh(const std::shared_ptr<Chunk>& chunk) {
    if (!chunk) {
        return;
    }

    ChunkPosition position{ chunk->GetChunkX(), chunk->GetChunkZ() };
    auto& entry = m_cache[position];

    if (entry.revision == chunk->GetRevision()) {
        return;
    }

    const ChunkMeshData meshData = m_mesher.BuildMesh(*chunk);

    UploadSection(entry.opaque, meshData.opaqueVertices, meshData.opaqueIndices);
    UploadSection(entry.transparent, meshData.transparentVertices, meshData.transparentIndices);

    entry.revision = chunk->GetRevision();
}

void ChunkMeshCache::RemoveChunk(const ChunkPosition& position) {
    auto it = m_cache.find(position);
    if (it != m_cache.end()) {
        m_cache.erase(it);
    }
}

void ChunkMeshCache::Clear() {
    m_cache.clear();
}

const ChunkMesh* ChunkMeshCache::GetChunkMesh(const ChunkPosition& position) const noexcept {
    const auto it = m_cache.find(position);
    if (it == m_cache.end()) {
        return nullptr;
    }

    return &it->second;
}

} // namespace CodexCraft::World::Meshing

