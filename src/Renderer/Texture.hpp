#pragma once

#include <string>

namespace renderer
{
class Texture
{
  public:
    Texture() = default;
    ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;

    bool load_atlas(const std::string& path, int expectedTilesX, int expectedTilesY);
    void bind(unsigned slot = 0) const;

    int width() const { return m_width; }
    int height() const { return m_height; }

  private:
    unsigned m_handle = 0;
    int m_width = 0;
    int m_height = 0;
};

} // namespace renderer
