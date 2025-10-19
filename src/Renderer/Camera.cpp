#include "Camera.hpp"

#include "Util/Math.hpp"

#include <glm/gtc/constants.hpp>

namespace renderer
{
Camera::Camera()
{
    set_perspective(m_fov, m_aspect, m_near, m_far);
}

void Camera::set_perspective(float fovRadians, float aspect, float nearPlane, float farPlane)
{
    m_fov = fovRadians;
    m_aspect = aspect;
    m_near = nearPlane;
    m_far = farPlane;
    m_projection = util::perspective(fovRadians, aspect, nearPlane, farPlane);
}

void Camera::resize(int width, int height)
{
    m_aspect = static_cast<float>(width) / static_cast<float>(height);
    set_perspective(m_fov, m_aspect, m_near, m_far);
}

void Camera::move(const glm::vec3& delta)
{
    m_position += delta;
}

void Camera::set_position(const glm::vec3& position)
{
    m_position = position;
}

void Camera::set_rotation(float pitch, float yaw)
{
    m_pitch = pitch;
    m_yaw = yaw;
}

void Camera::add_rotation(float pitchDelta, float yawDelta)
{
    m_pitch = glm::clamp(m_pitch + pitchDelta, -glm::half_pi<float>() + 0.001f, glm::half_pi<float>() - 0.001f);
    m_yaw += yawDelta;
}

glm::mat4 Camera::view() const
{
    return util::look_at(m_position, m_position + forward(), glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::vec3 Camera::forward() const
{
    return util::direction_from_euler(m_pitch, m_yaw);
}

} // namespace renderer
