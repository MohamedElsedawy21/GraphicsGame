cmake_minimum_required(VERSION 3.20)
project(CairoDefense LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- GLFW: downloaded automatically from GitHub ----
include(FetchContent)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)   # Linux only: use X11, fewer packages needed
FetchContent_MakeAvailable(glfw)

# ---- GLAD: the files you downloaded ----
add_library(glad STATIC external/glad/src/glad.c)
target_include_directories(glad PUBLIC external/glad/include)

# ---- Our game ----
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)
add_executable(game ${SOURCES})
target_include_directories(game PRIVATE
    src
    external/stb          # stb_easy_font.h (text)
    external/miniaudio    # miniaudio.h (sound)
)
target_link_libraries(game PRIVATE glfw glad)

# miniaudio is a huge third-party file: hide its warnings
set_source_files_properties(src/engine/AudioImpl.cpp PROPERTIES COMPILE_OPTIONS "-w")

# Extra system libraries miniaudio needs on Linux and Mac (Windows needs none)
if(UNIX AND NOT APPLE)
    target_link_libraries(game PRIVATE dl pthread m)
elseif(APPLE)
    target_link_libraries(game PRIVATE "-framework CoreAudio" "-framework AudioToolbox" "-framework CoreFoundation")
endif()
