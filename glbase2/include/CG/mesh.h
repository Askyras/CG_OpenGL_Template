#pragma once

#include <array>
#include <memory>
#include <vector>

#include "GL/glew.h"
#include "glm/glm.hpp"
namespace glm { using ubvec3 = glm::vec<3, unsigned char, glm::highp>; }


namespace CG
{
    enum BufferIndex : unsigned int {
        BufferIndex_Positions = 0,
        BufferIndex_Normals = 1,
        BufferIndex_Texcoords = 2,
        BufferIndex_Colors = 3,
        BufferIndex_Indices = 4
    };

    // RAII wrapper for an OpenGL vertex array object and attached buffers. Handles move semantics and deletion, but not construction.
    // See MeshBuilder::build() for an example of how to initialize the buffers.
    class Mesh {
    public:
        Mesh() = default;
        Mesh(const Mesh&) = delete;
        Mesh& operator=(const Mesh&) = delete;
        Mesh(Mesh&& other) noexcept;
        Mesh& operator=(Mesh&& other) noexcept;
        ~Mesh();

        GLuint vao = 0;
        GLsizei index_count = 0;
        std::array<GLuint, 5> vbos = { 0, 0, 0, 0, 0 };
        GLenum draw_mode = GL_TRIANGLES;
    };

    // Factory class for constructing Mesh objects from vertex data.
    // Mainly useful for compatibility with geomload.h and geometries.h functions, which output into existing vectors.
    class MeshBuilder {
    public:
        std::vector<glm::vec3> positions;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> texcoords;
        std::vector<glm::ubvec3> colors;
        std::vector<unsigned int> indices;

        std::shared_ptr<Mesh> build() const;
    };
}