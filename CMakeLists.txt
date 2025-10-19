cmake_minimum_required(VERSION 3.20)
project(CodexCraft LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.3.8
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG v0.1.36
)
set(GLAD_PROFILE "core" CACHE STRING "" FORCE)
set(GLAD_API "gl=4.5" CACHE STRING "" FORCE)
set(GLAD_GENERATOR "c" CACHE STRING "" FORCE)
FetchContent_MakeAvailable(glad)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 0.9.9.8
)
FetchContent_MakeAvailable(glm)

FetchContent_Declare(
    fastnoise
    GIT_REPOSITORY https://github.com/Auburn/FastNoiseLite.git
    GIT_TAG 1.1.0
)
FetchContent_MakeAvailable(fastnoise)

FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG master
)
FetchContent_MakeAvailable(stb)

file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS src/*.cpp)
file(GLOB_RECURSE PROJECT_HEADERS CONFIGURE_DEPENDS src/*.hpp src/*.h)

add_executable(CodexCraft ${PROJECT_SOURCES} ${PROJECT_HEADERS})

add_library(stb_image INTERFACE)
target_include_directories(stb_image INTERFACE ${stb_SOURCE_DIR})

find_package(OpenGL REQUIRED)

if (OPENGL_FOUND)
    target_link_libraries(CodexCraft PRIVATE OpenGL::GL)
endif()

target_include_directories(CodexCraft PRIVATE
    src
    ${glm_SOURCE_DIR}
    ${fastnoise_SOURCE_DIR}/Cpp
)

target_link_libraries(CodexCraft PRIVATE glfw glad glm FastNoiseLite stb_image)

if (MSVC)
    target_compile_options(CodexCraft PRIVATE /W4 /WX /permissive- /MP)
else()
    target_compile_options(CodexCraft PRIVATE -Wall -Wextra -Wpedantic -Werror)
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(CodexCraft PRIVATE -Wthread-safety)
    endif()
endif()

add_custom_target(copy_assets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        ${CMAKE_BINARY_DIR}/assets)
add_dependencies(CodexCraft copy_assets)

add_custom_target(copy_shaders ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/shaders
        ${CMAKE_BINARY_DIR}/shaders)
add_dependencies(CodexCraft copy_shaders)

if (MSVC)
    target_compile_definitions(CodexCraft PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

source_group(TREE ${CMAKE_SOURCE_DIR}/src FILES ${PROJECT_SOURCES} ${PROJECT_HEADERS})
