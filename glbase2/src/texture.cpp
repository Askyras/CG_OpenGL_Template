#include "CG/texture.h"
#include "CG/utils.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"


CG::Texture::Texture(Texture&& other) noexcept
    : tex_id(other.tex_id), target(other.target)
{
    other.tex_id = 0;
}

CG::Texture& CG::Texture::operator=(Texture&& other) noexcept
{
    if (this != &other) {
        if (tex_id != 0) {
            glDeleteTextures(1, &tex_id);
        }
        tex_id = other.tex_id;
        target = other.target;
        other.tex_id = 0;
    }
    return *this;
}

CG::Texture::~Texture()
{
    if (tex_id != 0) {
        glDeleteTextures(1, &tex_id);
        tex_id = 0;
    }
}


std::shared_ptr<CG::Texture> CG::load_texture(const std::string& resource_path, TextureLoadFlags flags, GLenum target)
{
    auto path = resolve_resource_path(resource_path);
    if (!path)
        return nullptr;

    auto tex = std::make_shared<CG::Texture>();
    tex->target = target;

    stbi_set_flip_vertically_on_load((flags & TextureFlags_FlipVertically) != 0);

    int width, height, components;
    unsigned char* pixels = stbi_load(path->string().c_str(), &width, &height, &components, 0);
    if (!pixels) {
        printf("Failed to load texture: %s\n", resource_path.c_str());
        return tex;
    }

    glGenTextures(1, &tex->tex_id);
    glBindTexture(target, tex->tex_id);

    // stb_image pixels are tightly packed, OpenGL may assume a larger row alignment.
    GLint previous_unpack_alignment;
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &previous_unpack_alignment);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    GLenum format = components == 4 ? GL_RGBA : components == 3 ? GL_RGB : components == 2 ? GL_RG : GL_RED;
    glTexImage2D(target, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, pixels);
    stbi_image_free(pixels);

    glPixelStorei(GL_UNPACK_ALIGNMENT, previous_unpack_alignment);

    if (flags & TextureFlags_GenerateMipmap)
        glGenerateMipmap(target);

    return tex;
}

bool CG::save_texture_png(GLenum target, const std::string& path, bool flip_vertically)
{
    // Check texture target is valid for saving
    switch (target) {
    case GL_TEXTURE_2D:
    case GL_TEXTURE_RECTANGLE:
    case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
    case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
    case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
        break;
    default:
        return false;
    }

    // Get texture size and format
    GLint width, height, internal_format;
    glGetTexLevelParameteriv(target, 0, GL_TEXTURE_WIDTH, &width);
    glGetTexLevelParameteriv(target, 0, GL_TEXTURE_HEIGHT, &height);
    glGetTexLevelParameteriv(target, 0, GL_TEXTURE_INTERNAL_FORMAT, &internal_format);
    if (width <= 0 || height <= 0 || internal_format <= 0)
        return false;
    int components = 0;
    switch (internal_format) {
    case GL_DEPTH_COMPONENT:
    case GL_LUMINANCE:
    case GL_RED:
    case GL_R8:
    case GL_R8_SNORM:
    case GL_R16:
    case GL_R16_SNORM:
    case GL_R16F:
    case GL_R32F:
    case GL_R8I:
    case GL_R8UI:
    case GL_R16I:
    case GL_R16UI:
    case GL_R32I:
    case GL_R32UI:
        components = 1;
        break;
    case GL_LUMINANCE_ALPHA:
    case GL_RG:
    case GL_RG8:
    case GL_RG8_SNORM:
    case GL_RG16:
    case GL_RG16_SNORM:
    case GL_RG16F:
    case GL_RG32F:
    case GL_RG8I:
    case GL_RG8UI:
    case GL_RG16I:
    case GL_RG16UI:
    case GL_RG32I:
    case GL_RG32UI:
        components = 2;
        break;
    case GL_R3_G3_B2:
    case GL_RGB4:
    case GL_RGB5:
    case GL_RGB10:
    case GL_RGB12:
    case GL_SRGB:
    case GL_SRGB8:
    case GL_R11F_G11F_B10F:
    case GL_RGB9_E5:
    case GL_RGB:
    case GL_RGB8:
    case GL_RGB8_SNORM:
    case GL_RGB16:
    case GL_RGB16_SNORM:
    case GL_RGB16F:
    case GL_RGB32F:
    case GL_RGB8I:
    case GL_RGB8UI:
    case GL_RGB16I:
    case GL_RGB16UI:
    case GL_RGB32I:
    case GL_RGB32UI:
        components = 3;
        break;
    case GL_RGBA4:
    case GL_RGB5_A1:
    case GL_RGB10_A2:
#ifdef GL_RGB10_A2UI
    case GL_RGB10_A2UI:
#endif
    case GL_RGBA12:
    case GL_SRGB_ALPHA:
    case GL_SRGB8_ALPHA8:
    case GL_RGBA:
    case GL_RGBA8:
    case GL_RGBA8_SNORM:
    case GL_RGBA16:
    case GL_RGBA16_SNORM:
    case GL_RGBA16F:
    case GL_RGBA32F:
    case GL_RGBA8I:
    case GL_RGBA8UI:
    case GL_RGBA16I:
    case GL_RGBA16UI:
    case GL_RGBA32I:
    case GL_RGBA32UI:
        components = 4;
        break;
    default: break;
    }
    if (components <= 0)
        return false;
    GLenum format = components == 4 ? GL_RGBA : components == 3 ? GL_RGB : components == 2 ? GL_RG : GL_RED;

    // Get texture pixels
    int row_size = width * components;
    std::vector<unsigned char> pixels(row_size * height);

    GLint previous_pack_alignment;
    glGetIntegerv(GL_PACK_ALIGNMENT, &previous_pack_alignment);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    glGetError(); // clear
    glGetTexImage(target, 0, format, GL_UNSIGNED_BYTE, pixels.data());
    if (glGetError() != GL_NO_ERROR)
        return false;

    glPixelStorei(GL_PACK_ALIGNMENT, previous_pack_alignment);

    // Write texture
    if (flip_vertically) {
        std::vector<unsigned char> temp_row(row_size);
        for (int y = 0; y < height / 2; y++) {
            auto* row0 = pixels.data() + y * row_size;
            auto* row1 = pixels.data() + (height - 1 - y) * row_size;
            std::copy(row0, row0 + row_size, temp_row.data());
            std::copy(row1, row1 + row_size, row0);
            std::copy(temp_row.data(), temp_row.data() + row_size, row1);
        }
    }
    return stbi_write_png(path.c_str(), width, height, components, pixels.data(), row_size) != 0;
}