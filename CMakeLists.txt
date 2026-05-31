# Minimum CMake version required. 3.20 added better presets support.
cmake_minimum_required(VERSION 3.20)

# vcpkg integration — finds libraries installed by vcpkg.
# CMAKE_TOOLCHAIN_FILE tells CMake how to find packages.
# ENV{VCPKG_ROOT} reads the environment variable we set earlier.
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE
        "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "Vcpkg toolchain file")
    message(STATUS "vcpkg toolchain: ${CMAKE_TOOLCHAIN_FILE}")
else()
    message(STATUS "CMAKE_TOOLCHAIN_FILE already set or VCPKG_ROOT not found")
    message(STATUS "CMAKE_TOOLCHAIN_FILE = ${CMAKE_TOOLCHAIN_FILE}")
    message(STATUS "VCPKG_ROOT = $ENV{VCPKG_ROOT}")
endif()

# Project name and version. VERSION is optional but good practice.
project(HeroAscendant VERSION 0.1.0)

# Tell CMake we want C++17 for this project only.
# REQUIRED means CMake will error if the compiler doesn't support it.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Disable compiler-specific extensions (e.g. GNU extensions).
# This keeps your code strictly standard C++17, more portable.
set(CMAKE_CXX_EXTENSIONS OFF)

# --- Find Libraries ---
# find_package looks for a library installed by vcpkg.
# CONFIG means use the library's own CMake config file.
# REQUIRED means error out if not found.
find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_image CONFIG REQUIRED)
find_package(SDL2_mixer CONFIG REQUIRED)
find_package(SDL2_ttf CONFIG REQUIRED)

# --- Gather Source Files ---
# GLOB_RECURSE finds ALL .cpp files inside src/ automatically.
# This means adding a new .cpp file to src/ = it's included on next CMake run.
# No need to manually list every file.
file(GLOB_RECURSE SOURCES "src/*.cpp")

# --- Create the Executable ---
add_executable(HeroAscendant ${SOURCES})

# --- Include Directories ---
# Tells the compiler where to look for #include files.
# src/ is the root, so #include "core/Game.h" works from anywhere.
target_include_directories(HeroAscendant PRIVATE src)

# --- Link Libraries ---
# Connects the installed SDL2 libraries to our executable.
# SDL2::SDL2main handles the WinMain vs main difference on Windows.
target_link_libraries(HeroAscendant
    PRIVATE
    SDL2::SDL2
    SDL2::SDL2main
    SDL2_image::SDL2_image
    SDL2_mixer::SDL2_mixer
    SDL2_ttf::SDL2_ttf
)

# --- Copy Assets ---
# After building, copies your entire assets/ folder next to the .exe
# so SDL_GetBasePath() + "assets/" always resolves correctly.
add_custom_command(TARGET HeroAscendant POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:HeroAscendant>/assets
    COMMENT "Copying assets..."
)

# --- Windows specific: copy SDL2 DLLs next to the .exe ---
# On Windows, SDL2 is a DLL. It must be in the same folder as the .exe.
# vcpkg provides a helper command that does this automatically.
if(WIN32)
    add_custom_command(TARGET HeroAscendant POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_RUNTIME_DLLS:HeroAscendant>
        $<TARGET_FILE_DIR:HeroAscendant>
        COMMENT "Copying SDL2 DLLs..."
        COMMAND_EXPAND_LISTS
    )
endif()