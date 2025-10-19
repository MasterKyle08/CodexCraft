#include "GLContext.hpp"

#include "Util/Logging.hpp"

#include <glad/glad.h>

namespace renderer
{
void GLContext::Deleter::operator()(GLFWwindow* window) const noexcept
{
    if (window)
    {
        glfwDestroyWindow(window);
    }
}

GLContext GLContext::create(int width, int height, bool fullscreen, const std::string& title)
{
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);

    GLFWmonitor* monitor = fullscreen ? glfwGetPrimaryMonitor() : nullptr;
    GLFWwindow* rawWindow = glfwCreateWindow(width, height, title.c_str(), monitor, nullptr);
    if (!rawWindow)
    {
        util::log().error("Failed to create GLFW window");
        return {};
    }

    glfwMakeContextCurrent(rawWindow);
    glfwSwapInterval(0);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        util::log().error("Failed to load GL functions");
        return {};
    }

    util::log().info("OpenGL %s", glGetString(GL_VERSION));
    return GLContext{WindowPtr(rawWindow)};
}

} // namespace renderer
