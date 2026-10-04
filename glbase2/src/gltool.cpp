#include <cstdio>

#include <GL/glew.h>

#include "CG/gltool.h"

#include <vector>


bool CG::checkFbo()
{
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
	std::fprintf(stderr, "OpenGL FBO error 0x%04X\n", status);
	return false;
    }
    return true;
}


void CG::dump_uniform_block_layout(GLuint prg, const char* block_name)
{
    const GLuint block = glGetUniformBlockIndex(prg, block_name);
    if (block == GL_INVALID_INDEX) {
        printf("Block '%s' not found\n", block_name);
        return;
    }

    GLint numUniforms = 0;
    glGetActiveUniformBlockiv(prg, block, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &numUniforms);

    std::vector<GLint> indices(numUniforms);
    glGetActiveUniformBlockiv(prg, block, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, indices.data());

    printf("Uniform block '%s': %d active uniforms\n", block_name, numUniforms);

    for (GLint i = 0; i < numUniforms; ++i) {
        GLuint index = static_cast<GLuint>(indices[i]);

        GLint nameLength = 0;
        glGetActiveUniformsiv(prg, 1, &index, GL_UNIFORM_NAME_LENGTH, &nameLength);

        std::vector<char> name(nameLength);

        GLsizei actualLength = 0;
        GLint size = 0;
        GLenum type = 0;
        GLint offset = -1;
        GLint arrayStride = -1;
        GLint matrixStride = -1;
        GLint rowMajor = GL_FALSE;
        glGetActiveUniform(prg, index, nameLength, &actualLength, &size, &type, name.data());
        glGetActiveUniformsiv(prg, 1, &index, GL_UNIFORM_OFFSET, &offset);
        glGetActiveUniformsiv(prg, 1, &index, GL_UNIFORM_ARRAY_STRIDE, &arrayStride);
        glGetActiveUniformsiv(prg, 1, &index, GL_UNIFORM_MATRIX_STRIDE, &matrixStride);
        glGetActiveUniformsiv(prg, 1, &index, GL_UNIFORM_IS_ROW_MAJOR, &rowMajor);

        printf("%-30s offset=%3d size=%d arrayStride=%3d matrixStride=%3d rowMajor=%d type=0x%x\n",
            name.data(), offset, size, arrayStride, matrixStride, rowMajor, type
        );
    }
}