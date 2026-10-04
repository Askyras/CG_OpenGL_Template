cmake_minimum_required(VERSION 3.16)

project(glew LANGUAGES C)

# GLEW source root.
set(GLEW_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../..)

# Options retained from the original build where they affect the
# actual GLEW compilation/platform integration.
option(GLEW_REGAL  "Regal mode" OFF)
option(GLEW_OSMESA "OSMesa mode" OFF)
option(GLEW_X11   "X11 mode" ON)
option(GLEW_EGL   "EGL mode" OFF)

# ---------------------------------------------------------------------------
# OpenGL / platform dependencies
# ---------------------------------------------------------------------------

find_package(OpenGL REQUIRED)

# Prefer GLVND when available.
if (OPENGL_opengl_LIBRARY)
    set(GLEW_LIBRARIES ${OPENGL_opengl_LIBRARY})
else ()
    set(GLEW_LIBRARIES ${OPENGL_gl_LIBRARY})
endif ()

# X11 is required except on Windows and macOS.
if (GLEW_X11 AND NOT WIN32 AND NOT APPLE)
    find_package(X11 REQUIRED)
    list(APPEND GLEW_LIBRARIES
        ${OPENGL_glx_LIBRARY}
        ${X11_LIBRARIES}
    )
endif ()

# ---------------------------------------------------------------------------
# GLEW configuration
# ---------------------------------------------------------------------------

add_definitions(-DGLEW_NO_GLU)

if (GLEW_REGAL)
    if (WIN32)
        set(REGAL_LIB_NAME regal32)
    else ()
        set(REGAL_LIB_NAME Regal)
    endif ()

    add_definitions(-DGLEW_REGAL)
    set(GLEW_LIBRARIES ${REGAL_LIB_NAME})
endif ()

if (GLEW_OSMESA)
    if (WIN32)
        set(OSMESA_LIB_NAME osmesa)
    else ()
        set(OSMESA_LIB_NAME OSMesa)
    endif ()

    add_definitions(-DGLEW_OSMESA)
    list(APPEND GLEW_LIBRARIES ${OSMESA_LIB_NAME})

    # OSMesa does not use X11.
    set(X11_LIBRARIES)
endif ()

if (GLEW_EGL AND UNIX)
    add_definitions(-DGLEW_EGL)

    if (NOT OpenGL_EGL_FOUND)
        message(FATAL_ERROR "EGL library set but not found.")
    endif ()

    list(APPEND GLEW_LIBRARIES ${OPENGL_egl_LIBRARY})
endif ()

# ---------------------------------------------------------------------------
# GLEW static library
# ---------------------------------------------------------------------------

set(GLEW_PUBLIC_HEADERS
    ${GLEW_DIR}/include/GL/wglew.h
    ${GLEW_DIR}/include/GL/glew.h
    ${GLEW_DIR}/include/GL/glxew.h
    ${GLEW_DIR}/include/GL/eglew.h
)

set(GLEW_SOURCES
    ${GLEW_DIR}/src/glew.c
)

if (WIN32)
    list(APPEND GLEW_SOURCES
        ${GLEW_DIR}/build/glew.rc
    )
endif ()

add_library(glew STATIC
    ${GLEW_PUBLIC_HEADERS}
    ${GLEW_SOURCES}
)

set_target_properties(glew PROPERTIES
    OUTPUT_NAME glew
)

target_compile_definitions(glew
    PRIVATE GLEW_STATIC
)

target_include_directories(glew
    PUBLIC
        ${GLEW_DIR}/include
        ${X11_INCLUDE_DIR}
)

target_link_libraries(glew
    PUBLIC
        ${GLEW_LIBRARIES}
)

# ---------------------------------------------------------------------------
# Compiler/platform-specific configuration
# ---------------------------------------------------------------------------

if (MSVC)
    # Match the relevant options from the original Visual Studio project.
    target_compile_definitions(glew PRIVATE VC_EXTRALEAN)

    # Kill security checks which depend on the stdlib.
    target_compile_options(glew PRIVATE -GS-)

    # Remove stdlib dependency.
    #
    # These were originally applied to the DLL target. They are therefore
    # intentionally omitted here: a normal static library should retain the
    # standard runtime linkage of the consuming application.
    string(REGEX REPLACE
        "/RTC(su|[1su])"
        ""
        CMAKE_C_FLAGS_DEBUG
        "${CMAKE_C_FLAGS_DEBUG}"
    )

elseif (WIN32 AND
        (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang"))

    # Match the original Windows GCC/Clang configuration.
    target_compile_options(glew PRIVATE
        -fno-builtin
        -fno-stack-protector
    )

endif ()