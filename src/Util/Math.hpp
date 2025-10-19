#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace util
{
inline glm::mat4 perspective(float fovRadians, float aspect, float zNear, float zFar)
{
    return glm::perspective(fovRadians, aspect, zNear, zFar);
}

inline glm::mat4 look_at(const glm::vec3& pos, const glm::vec3& target, const glm::vec3& up)
{
    return glm::lookAt(pos, target, up);
}

inline glm::vec3 direction_from_euler(float pitch, float yaw)
{
    const glm::vec3 front{
        std::cos(pitch) * std::cos(yaw),
        std::sin(pitch),
        std::cos(pitch) * std::sin(yaw)};
    return glm::normalize(front);
}

} // namespace util
