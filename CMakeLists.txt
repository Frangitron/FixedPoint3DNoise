# Borrowed from https://github.com/ilyas-taouaou/CodotakuCMakeRepo/blob/master/CMakeLists.txt
cmake_minimum_required(VERSION 3.30)
project(FixedPointSimplex3DNoise)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# CPM
file(
        DOWNLOAD
        https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.38.3/CPM.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake
        EXPECTED_HASH SHA256=cc155ce02e7945e7b8967ddfaff0b050e958a723ef7aad3766d368940cb15494
)
include(${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake)

set(LIBS)


## Dependencies

# ImGui
CPMAddPackage(
        NAME imgui
        GITHUB_REPOSITORY ocornut/imgui
        GIT_TAG v1.91.9b
        DOWNLOAD_ONLY
)
if (imgui_ADDED)
    add_library(imgui STATIC)
    target_sources(imgui PRIVATE
            ${imgui_SOURCE_DIR}/imgui.cpp
            ${imgui_SOURCE_DIR}/imgui_draw.cpp
            ${imgui_SOURCE_DIR}/imgui_tables.cpp
            ${imgui_SOURCE_DIR}/imgui_widgets.cpp
            ${imgui_SOURCE_DIR}/imgui_demo.cpp
            # SDL 3
            ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
            # SDL 3 Renderer
            ${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer3.cpp
    )
    target_include_directories(imgui PUBLIC "${imgui_SOURCE_DIR}")
    target_include_directories(imgui PUBLIC "${imgui_SOURCE_DIR}/backends")
    list(APPEND LIBS imgui)
endif ()

# SDL
CPMAddPackage("gh:libsdl-org/SDL#release-3.2.8")
list(APPEND LIBS SDL3::SDL3)
if (imgui_ADDED)
    target_link_libraries(imgui PRIVATE SDL3::SDL3)
endif ()

## Executables
include_directories("include")
add_executable(FixedPointSimplex3DNoise src/main.cpp)
target_compile_features(FixedPointSimplex3DNoise PRIVATE cxx_std_23)
target_link_libraries(FixedPointSimplex3DNoise PRIVATE ${LIBS})
