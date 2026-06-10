cmake_minimum_required(VERSION 3.15)

project(computer_graphics VERSION 1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG 3.4
)

FetchContent_MakeAvailable(glfw)
find_package(OpenGL REQUIRED)


add_executable(
    computer_graphics

    src/main.cpp
    src/application.cpp
    src/camera.cpp
    src/scene.cpp
    src/shader.cpp
    src/worldmesh.cpp
    src/watermesh.cpp
    src/skydome.cpp

    include/src/glad.c
)

target_include_directories(
    computer_graphics
    PRIVATE
    headers
    include
)

target_link_libraries(
    computer_graphics
    PRIVATE
    glfw
    OpenGL::GL
)