#include "Texture.hpp"

#include "Util/Logging.hpp"

#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace renderer
{
Texture::~Texture()
{
    if (m_handle)
    {
        glDeleteTextures(1, &m_handle);
    }
}

Texture::Texture(Texture&& other) noexcept
{
    *this = std::move(other);
}

Texture& Texture::operator=(Texture&& other) noexcept
{
    if (this != &other)
    {
        std::swap(m_handle, other.m_handle);
        std::swap(m_width, other.m_width);
        std::swap(m_height, other.m_height);
    }
    return *this;
}

bool Texture::load_atlas(const std::string& path, int expectedTilesX, int expectedTilesY)
{
    stbi_set_flip_vertically_on_load(false);
    int channels = 0;
    unsigned char* data = stbi_load(path.c_str(), &m_width, &m_height, &channels, STBI_rgb_alpha);
    if (!data)
    {
        util::log().error("Failed to load atlas %s", path.c_str());
        return false;
    }

    if ((m_width % expectedTilesX) != 0 || (m_height % expectedTilesY) != 0)
    {
        util::log().warn("Atlas size %dx%d does not align with %dx%d grid", m_width, m_height, expectedTilesX, expectedTilesY);
    }

    if (!m_handle)
    {
        glCreateTextures(GL_TEXTURE_2D, 1, &m_handle);
    }
    glTextureParameteri(m_handle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_handle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_handle, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTextureParameteri(m_handle, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTextureStorage2D(m_handle, 1, GL_RGBA8, m_width, m_height);
    glTextureSubImage2D(m_handle, 0, 0, 0, m_width, m_height, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateTextureMipmap(m_handle);

    stbi_image_free(data);
    return true;
}

void Texture::bind(unsigned slot) const
{
    glBindTextureUnit(slot, m_handle);
}

} // namespace renderer
