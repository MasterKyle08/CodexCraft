#pragma once

#include <array>
#include <glm/glm.hpp>

namespace renderer
{
class Frustum
{
  public:
    void update(const glm::mat4& projectionView);
    bool intersects(const glm::vec3& min, const glm::vec3& max) const;

  private:
    std::array<glm::vec4, 6> m_planes{};
};

} // namespace renderer
