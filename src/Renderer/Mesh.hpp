#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace renderer
{
struct ChunkVertex
{
    float position[3];
    std::uint32_t normalPacked;
    float uv[2];
    std::uint8_t light;
    std::uint8_t padding[3]{}; // Align to 4 bytes for std140 friendly layout.
};

class Mesh
{
  public:
    Mesh();
    ~Mesh();

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;

    void upload(const std::vector<ChunkVertex>& vertices, const std::vector<std::uint32_t>& indices, bool dynamic = false);
    void draw() const;
    bool empty() const { return m_indexCount == 0; }

  private:
    void destroy();

    unsigned m_vao = 0;
    unsigned m_vbo = 0;
    unsigned m_ibo = 0;
    std::size_t m_vertexCapacity = 0;
    std::size_t m_indexCapacity = 0;
    std::uint32_t m_indexCount = 0;
    bool m_dynamic = false;
};

} // namespace renderer
