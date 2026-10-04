#pragma once

#include "GL/glew.h"


namespace CG
{
    // RAII wrapper for an OpenGL buffer object. Handles move semantics and deletion, but not construction.
    // Can be used directly for GL calls in place of the underlying GLuint due to the implicit conversion operator.
    class BufferResource {
    public:
        GLuint handle = 0;

        BufferResource() = default;
        BufferResource(const BufferResource&) = delete;
        BufferResource& operator=(const BufferResource&) = delete;
        BufferResource(BufferResource&& other) noexcept
            : handle(other.handle) {
            other.handle = 0;
        }
        BufferResource& operator=(BufferResource&& other) noexcept
        {
            if (this != &other) {
                if (handle != 0)
                    glDeleteBuffers(1, &handle);
                handle = other.handle;
                other.handle = 0;
            }
            return *this;
        }

        ~BufferResource() {
            if (handle != 0) {
                glDeleteBuffers(1, &handle);
                handle = 0;
            }
        }

        operator GLuint() const { return handle; }
    };
}