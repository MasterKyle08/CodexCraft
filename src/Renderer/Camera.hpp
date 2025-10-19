#pragma once

#include <glm/glm.hpp>

namespace renderer
{
class Camera
{
  public:
    Camera();

    void set_perspective(float fovRadians, float aspect, float nearPlane, float farPlane);
    void resize(int width, int height);

    void move(const glm::vec3& delta);
    void set_position(const glm::vec3& position);
    void set_rotation(float pitch, float yaw);
    void add_rotation(float pitchDelta, float yawDelta);

    glm::mat4 view() const;
    glm::mat4 projection() const { return m_projection; }
    glm::vec3 position() const { return m_position; }
    glm::vec3 forward() const;

  private:
    glm::vec3 m_position{0.0f};
    float m_pitch = 0.0f;
    float m_yaw = 0.0f;

    float m_fov = glm::radians(70.0f);
    float m_aspect = 16.0f / 9.0f;
    float m_near = 0.1f;
    float m_far = 1000.0f;
    glm::mat4 m_projection{1.0f};
};

} // namespace renderer
