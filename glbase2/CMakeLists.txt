cmake_minimum_required (VERSION 3.24)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_NAME "glbase2")
project(${PROJECT_NAME} CXX)

find_package(OpenGL REQUIRED)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(SDL_INSTALL OFF CACHE BOOL "" FORCE)
set(SDL_UNINSTALL OFF CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(HIDAPI_WITH_TESTS OFF CACHE BOOL "" FORCE)
set(HIDAPI_BUILD_HIDTEST OFF CACHE BOOL "" FORCE)
set(HIDAPI_BUILD_PP_DATA_DUMP OFF CACHE BOOL "" FORCE)
add_subdirectory("vendor/SDL3")

add_subdirectory("vendor/FastNoise2")

add_subdirectory("vendor/glew/build/cmake")

add_subdirectory("vendor/glm")

add_subdirectory("vendor/imgui")
target_include_directories(imgui PUBLIC "vendor/glm")

add_library (${PROJECT_NAME} STATIC
	"src/application.cpp"
	"src/geometries.cpp"
	"src/geomload.cpp"
	"src/gltool.cpp"
	"src/mesh.cpp"
	"src/shader.cpp" 
	
	"src/texture.cpp"

	"vendor/tinyobjloader/tiny_obj_loader.cpp"
	"vendor/ply/plyfile.cpp"
)

target_compile_definitions(${PROJECT_NAME} PUBLIC 
	NOMINMAX
	GLEW_STATIC
	GLM_ENABLE_EXPERIMENTAL
	CG_RESOURCE_PATH=\"${CG_RESOURCE_PATH}\"
)

target_include_directories(${PROJECT_NAME} PRIVATE "src")
target_include_directories(${PROJECT_NAME} PUBLIC
	"include"
	"vendor"
	"vendor/lodepng"
	"vendor/tinyobjloader"
	"vendor/ply"
    "vendor/glew/include"
	"vendor/glm"
	"vendor/imgui"
    "vendor/SDL3/include"
	${OPENGL_INCLUDE_DIR}
)

target_link_libraries(${PROJECT_NAME} PUBLIC
	FastNoise
	glew
	imgui
    SDL3::SDL3
    ${OPENGL_gl_LIBRARY}
)
