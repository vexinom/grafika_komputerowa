cmake_minimum_required(VERSION 3.15)

set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)

project(computer_graphics LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

include(FetchContent)

FetchContent_Declare(glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
)
FetchContent_MakeAvailable(glm)

FetchContent_Declare(glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
)
FetchContent_MakeAvailable(glad)

find_package(OpenGL REQUIRED)

add_executable(computer_graphics
    src/main.cpp
    src/objloader.cpp
    src/application.cpp
    src/camera.cpp
    src/scene.cpp
    src/shader.cpp
    src/worldmesh.cpp
    src/watermesh.cpp
    src/skydome.cpp
    src/cubemap.cpp
    src/tube.cpp
    src/monument.cpp
    src/axolotl.cpp
    src/fish.cpp
    src/otter.cpp
    src/reef.cpp
    src/seaweed.cpp
    src/particles.cpp
    src/waterFrameBuffers.cpp
)

target_include_directories(computer_graphics PRIVATE
    headers
)

target_link_libraries(computer_graphics PRIVATE
    glfw
    glad
    glm::glm
    OpenGL::GL
)

add_custom_command(TARGET computer_graphics POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/shaders
        $<TARGET_FILE_DIR:computer_graphics>/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:computer_graphics>/assets
)
