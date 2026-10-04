#pragma once

#include "GL/glew.h"

#include <memory>
#include <string>


namespace CG
{
    // RAII wrapper for an OpenGL texture object. Handles move semantics and deletion, but not construction.
    // Can be used directly for GL calls in place of the underlying GLuint due to the implicit conversion operator.
    class Texture {
    public:
        Texture() = default;
        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;
        Texture(Texture&& other) noexcept;
        Texture& operator=(Texture&& other) noexcept;
        ~Texture();

        GLuint tex_id = 0;
        GLenum target = GL_TEXTURE_2D;

        operator GLuint() const { return tex_id; }
    };

    enum TextureLoadFlags : unsigned int {
        TextureFlags_None = 0,
        TextureFlags_FlipVertically = 1 << 0,
        TextureFlags_GenerateMipmap = 1 << 1,
        TextureFlags_Default = TextureFlags_FlipVertically | TextureFlags_GenerateMipmap
    };

    // Load a texture file using stb_image from the given path (relative to the resource directory) and upload it to an OpenGL texture.
    std::shared_ptr<CG::Texture> load_texture(const std::string& resource_path, TextureLoadFlags flags = TextureFlags_Default, GLenum target = GL_TEXTURE_2D);

    // Save the texture currently bound to the given target to a PNG file at the given path (absolute, or relative to working directory).
    bool save_texture_png(GLenum target, const std::string& path, bool flip_vertically = true);
}