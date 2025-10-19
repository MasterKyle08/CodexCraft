#pragma once

#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif
#include <GLFW/glfw3.h>
#include <memory>
#include <string>
#include <utility>

namespace renderer
{
class GLContext
{
  public:
    GLContext() = default;
    explicit GLContext(WindowPtr window) : m_window(std::move(window)) {}

    struct Deleter
    {
        void operator()(GLFWwindow* window) const noexcept;
    };

    using WindowPtr = std::unique_ptr<GLFWwindow, Deleter>;

    static GLContext create(int width, int height, bool fullscreen, const std::string& title);

    GLFWwindow* window() const { return m_window.get(); }

  private:
    WindowPtr m_window;
};

} // namespace renderer
