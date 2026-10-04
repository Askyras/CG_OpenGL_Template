#pragma once

#include <cstdio>

namespace CG 
{
    bool checkFbo();

    void dump_uniform_block_layout(GLuint prg, const char* block_name);
}

// A macro VERIFY(condition) for quick-and-dirty error checking: if the
// condition is not met, the program aborts immediately with an error message.
#define VERIFY(condition)  \
    if (!(condition)) \
    { \
        fprintf(stderr, "%s:%d: Verification '%s' failed.\n", \
                __FILE__, __LINE__, #condition); \
        exit(-1); \
    }
