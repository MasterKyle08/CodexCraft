#pragma once

#include <glm/glm.hpp>
#include <string>

namespace renderer
{
class Shader
{
  public:
    Shader() = default;
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    ~Shader();

    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    bool load(const std::string& vertexPath, const std::string& fragmentPath);
    void use() const;
    void reload();

    void set_mat4(const char* name, const glm::mat4& value) const;
    void set_vec3(const char* name, const glm::vec3& value) const;
    void set_float(const char* name, float value) const;
    void set_int(const char* name, int value) const;

  private:
    unsigned compile_shader(unsigned type, const std::string& source);
    bool link_program(unsigned vert, unsigned frag);
    std::string read_file(const std::string& path);

    unsigned m_program = 0;
    std::string m_vertexPath;
    std::string m_fragmentPath;
};

} // namespace renderer
