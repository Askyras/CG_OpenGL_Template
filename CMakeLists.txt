cmake_minimum_required (VERSION 3.24)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_NAME "CG-Template")
project(${PROJECT_NAME} CXX)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

set(CG_RESOURCE_PATH "${CMAKE_SOURCE_DIR}/res" CACHE PATH "Path to the resources directory")
add_subdirectory("glbase2")

set(SHARED_SOURCES "")

function(add_glbase_example target_name)
    add_executable(${target_name} ${SHARED_SOURCES} ${ARGN})
    target_link_libraries(${target_name} PRIVATE glbase2)
    target_include_directories(${target_name} PUBLIC "src")
endfunction()

add_glbase_example(TemplateApp "src/template_app.cpp")
add_glbase_example(ExampleApp "src/editor_example.cpp")
