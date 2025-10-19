#include "Frustum.hpp"

#include <algorithm>

namespace renderer
{
void Frustum::update(const glm::mat4& projectionView)
{
    // Planes are extracted from the combined projection-view matrix.
    const glm::mat4& m = projectionView;
    m_planes[0] = glm::vec4(m[0][3] + m[0][0], m[1][3] + m[1][0], m[2][3] + m[2][0], m[3][3] + m[3][0]); // Left
    m_planes[1] = glm::vec4(m[0][3] - m[0][0], m[1][3] - m[1][0], m[2][3] - m[2][0], m[3][3] - m[3][0]); // Right
    m_planes[2] = glm::vec4(m[0][3] + m[0][1], m[1][3] + m[1][1], m[2][3] + m[2][1], m[3][3] + m[3][1]); // Bottom
    m_planes[3] = glm::vec4(m[0][3] - m[0][1], m[1][3] - m[1][1], m[2][3] - m[2][1], m[3][3] - m[3][1]); // Top
    m_planes[4] = glm::vec4(m[0][3] + m[0][2], m[1][3] + m[1][2], m[2][3] + m[2][2], m[3][3] + m[3][2]); // Near
    m_planes[5] = glm::vec4(m[0][3] - m[0][2], m[1][3] - m[1][2], m[2][3] - m[2][2], m[3][3] - m[3][2]); // Far

    for (auto& plane : m_planes)
    {
        const float length = glm::length(glm::vec3(plane));
        if (length > 0.0f)
        {
            plane /= length;
        }
    }
}

bool Frustum::intersects(const glm::vec3& min, const glm::vec3& max) const
{
    for (const auto& plane : m_planes)
    {
        glm::vec3 positiveVertex = min;
        if (plane.x >= 0)
            positiveVertex.x = max.x;
        if (plane.y >= 0)
            positiveVertex.y = max.y;
        if (plane.z >= 0)
            positiveVertex.z = max.z;

        if (glm::dot(glm::vec3(plane), positiveVertex) + plane.w < 0.0f)
        {
            return false;
        }
    }
    return true;
}

} // namespace renderer
