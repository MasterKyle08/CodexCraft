#include "Shader.hpp"

#include "Util/Logging.hpp"

#include <fstream>
#include <glad/glad.h>
#include <sstream>

namespace renderer
{
Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath)
{
    load(vertexPath, fragmentPath);
}

Shader::~Shader()
{
    if (m_program)
    {
        glDeleteProgram(m_program);
    }
}

Shader::Shader(Shader&& other) noexcept
{
    *this = std::move(other);
}

Shader& Shader::operator=(Shader&& other) noexcept
{
    if (this != &other)
    {
        std::swap(m_program, other.m_program);
        std::swap(m_vertexPath, other.m_vertexPath);
        std::swap(m_fragmentPath, other.m_fragmentPath);
    }
    return *this;
}

bool Shader::load(const std::string& vertexPath, const std::string& fragmentPath)
{
    m_vertexPath = vertexPath;
    m_fragmentPath = fragmentPath;

    auto vertSource = read_file(vertexPath);
    auto fragSource = read_file(fragmentPath);

    if (vertSource.empty() || fragSource.empty())
    {
        return false;
    }

    const auto vert = compile_shader(GL_VERTEX_SHADER, vertSource);
    const auto frag = compile_shader(GL_FRAGMENT_SHADER, fragSource);

    if (!vert || !frag)
    {
        return false;
    }

    if (m_program)
    {
        glDeleteProgram(m_program);
    }

    if (!link_program(vert, frag))
    {
        glDeleteShader(vert);
        glDeleteShader(frag);
        return false;
    }

    glDeleteShader(vert);
    glDeleteShader(frag);
    return true;
}

void Shader::use() const
{
    glUseProgram(m_program);
}

void Shader::reload()
{
    load(m_vertexPath, m_fragmentPath);
}

void Shader::set_mat4(const char* name, const glm::mat4& value) const
{
    glUniformMatrix4fv(glGetUniformLocation(m_program, name), 1, GL_FALSE, &value[0][0]);
}

void Shader::set_vec3(const char* name, const glm::vec3& value) const
{
    glUniform3fv(glGetUniformLocation(m_program, name), 1, &value[0]);
}

void Shader::set_float(const char* name, float value) const
{
    glUniform1f(glGetUniformLocation(m_program, name), value);
}

void Shader::set_int(const char* name, int value) const
{
    glUniform1i(glGetUniformLocation(m_program, name), value);
}

unsigned Shader::compile_shader(unsigned type, const std::string& source)
{
    unsigned shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    int success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        int len = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
        std::string log(len, '\0');
        glGetShaderInfoLog(shader, len, nullptr, log.data());
        util::log().error("Shader compile error: %s", log.c_str());
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

bool Shader::link_program(unsigned vert, unsigned frag)
{
    m_program = glCreateProgram();
    glAttachShader(m_program, vert);
    glAttachShader(m_program, frag);
    glLinkProgram(m_program);

    int success = 0;
    glGetProgramiv(m_program, GL_LINK_STATUS, &success);
    if (!success)
    {
        int len = 0;
        glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &len);
        std::string log(len, '\0');
        glGetProgramInfoLog(m_program, len, nullptr, log.data());
        util::log().error("Program link error: %s", log.c_str());
        glDeleteProgram(m_program);
        m_program = 0;
        return false;
    }
    return true;
}

std::string Shader::read_file(const std::string& path)
{
    std::ifstream file(path);
    if (!file)
    {
        util::log().error("Unable to open shader file %s", path.c_str());
        return {};
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

} // namespace renderer
