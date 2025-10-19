#include "Mesh.hpp"

#include <cstddef>

#include <glad/glad.h>

namespace renderer
{
Mesh::Mesh()
{
    glCreateVertexArrays(1, &m_vao);
    glCreateBuffers(1, &m_vbo);
    glCreateBuffers(1, &m_ibo);

    glVertexArrayVertexBuffer(m_vao, 0, m_vbo, 0, sizeof(ChunkVertex));
    glEnableVertexArrayAttrib(m_vao, 0);
    glVertexArrayAttribFormat(m_vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(ChunkVertex, position));
    glVertexArrayAttribBinding(m_vao, 0, 0);

    glEnableVertexArrayAttrib(m_vao, 1);
    glVertexArrayAttribIFormat(m_vao, 1, 1, GL_UNSIGNED_INT, offsetof(ChunkVertex, normalPacked));
    glVertexArrayAttribBinding(m_vao, 1, 0);

    glEnableVertexArrayAttrib(m_vao, 2);
    glVertexArrayAttribFormat(m_vao, 2, 2, GL_FLOAT, GL_FALSE, offsetof(ChunkVertex, uv));
    glVertexArrayAttribBinding(m_vao, 2, 0);

    glEnableVertexArrayAttrib(m_vao, 3);
    glVertexArrayAttribIFormat(m_vao, 3, 1, GL_UNSIGNED_BYTE, offsetof(ChunkVertex, light));
    glVertexArrayAttribBinding(m_vao, 3, 0);

    glVertexArrayElementBuffer(m_vao, m_ibo);
}

Mesh::~Mesh()
{
    destroy();
}

Mesh::Mesh(Mesh&& other) noexcept
{
    *this = std::move(other);
}

Mesh& Mesh::operator=(Mesh&& other) noexcept
{
    if (this != &other)
    {
        destroy();
        std::swap(m_vao, other.m_vao);
        std::swap(m_vbo, other.m_vbo);
        std::swap(m_ibo, other.m_ibo);
        std::swap(m_vertexCapacity, other.m_vertexCapacity);
        std::swap(m_indexCapacity, other.m_indexCapacity);
        std::swap(m_indexCount, other.m_indexCount);
        std::swap(m_dynamic, other.m_dynamic);
    }
    return *this;
}

void Mesh::destroy()
{
    if (m_vao)
    {
        glDeleteVertexArrays(1, &m_vao);
        glDeleteBuffers(1, &m_vbo);
        glDeleteBuffers(1, &m_ibo);
        m_vao = m_vbo = m_ibo = 0;
    }
}

void Mesh::upload(const std::vector<ChunkVertex>& vertices, const std::vector<std::uint32_t>& indices, bool dynamic)
{
    m_dynamic = dynamic;
    const auto vertexCount = vertices.size();
    const auto indexCount = indices.size();

    if (vertexCount > m_vertexCapacity)
    {
        glNamedBufferData(m_vbo, static_cast<GLsizeiptr>(vertexCount * sizeof(ChunkVertex)), vertices.data(), dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
        m_vertexCapacity = vertexCount;
    }
    else
    {
        glNamedBufferSubData(m_vbo, 0, static_cast<GLsizeiptr>(vertexCount * sizeof(ChunkVertex)), vertices.data());
    }

    if (indexCount > m_indexCapacity)
    {
        glNamedBufferData(m_ibo, static_cast<GLsizeiptr>(indexCount * sizeof(std::uint32_t)), indices.data(), dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
        m_indexCapacity = indexCount;
    }
    else
    {
        glNamedBufferSubData(m_ibo, 0, static_cast<GLsizeiptr>(indexCount * sizeof(std::uint32_t)), indices.data());
    }

    m_indexCount = static_cast<std::uint32_t>(indexCount);
}

void Mesh::draw() const
{
    if (!m_indexCount)
        return;

    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indexCount), GL_UNSIGNED_INT, nullptr);
}

} // namespace renderer
