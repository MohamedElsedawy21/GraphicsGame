cmake_minimum_required(VERSION 3.16)
project(MetroDefenders C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Template)

add_executable(game
    ${SRC_DIR}/main.cpp
    ${SRC_DIR}/renderer.cpp
    ${SRC_DIR}/models.cpp
    ${SRC_DIR}/game.cpp
    ${SRC_DIR}/menus.cpp
    ${SRC_DIR}/cutscene.cpp
    ${SRC_DIR}/audio.cpp
    ${SRC_DIR}/src/glad.c
)

target_include_directories(game PRIVATE
    ${SRC_DIR}
    ${SRC_DIR}/include
    ${SRC_DIR}/External/include
)

# ---------------------------------------------------------------- GLFW
if(MSVC)
    # Visual Studio compiler: use the prebuilt library from the course template
    target_link_libraries(game PRIVATE ${SRC_DIR}/lib/glfw3.lib opengl32)
elseif(WIN32)
    # MinGW g++: the template's glfw3.lib only works with MSVC, so build GLFW from source
    include(FetchContent)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(glfw
        URL https://github.com/glfw/glfw/releases/download/3.4/glfw-3.4.zip)
    FetchContent_MakeAvailable(glfw)
    target_link_libraries(game PRIVATE glfw opengl32 ole32)
    target_link_options(game PRIVATE -static-libgcc -static-libstdc++)
else()
    # Linux / macOS
    find_package(glfw3 REQUIRED)
    find_package(OpenGL REQUIRED)
    target_link_libraries(game PRIVATE glfw OpenGL::GL ${CMAKE_DL_LIBS} pthread m)
endif()

# put game.exe directly in build/ (also for Visual Studio's Debug/Release folders)
set_target_properties(game PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR})

# copy the sounds next to the executable
add_custom_command(TARGET game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${SRC_DIR}/sounds $<TARGET_FILE_DIR:game>/sounds)
