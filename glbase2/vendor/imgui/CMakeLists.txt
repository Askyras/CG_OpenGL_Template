cmake_minimum_required(VERSION 3.12)

project(ImGUI)

add_library(imgui STATIC
	"imconfig.h"
    "imgui.cpp"
	"imgui.h"
	"imgui_demo.cpp"
	"imgui_draw.cpp"
	"imgui_internal.h"
    "imgui_tables.cpp"
	"imgui_widgets.cpp"
	"imstb_rectpack.h"
	"imstb_textedit.h"
	"imstb_truetype.h"
	"misc/cpp/imgui_stdlib.cpp"
    #"misc/freetype/imgui_freetype.cpp"
    "backends/imgui_impl_sdl3.cpp"
    "backends/imgui_impl_opengl3.cpp"
	)
	
find_package(OpenGL REQUIRED)

target_include_directories(imgui PUBLIC 
	"${CMAKE_CURRENT_SOURCE_DIR}"
	"${CMAKE_CURRENT_SOURCE_DIR}/backends"
	"${CMAKE_CURRENT_SOURCE_DIR}/misc/cpp"
)

target_link_libraries(imgui PUBLIC
    ${OPENGL_gl_LIBRARY}
	SDL3::SDL3
)
