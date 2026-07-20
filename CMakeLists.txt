cmake_minimum_required(VERSION 3.16)
project(chip8_emulator C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Find SDL2 (works if SDL2 is installed system-wide, e.g. via apt/brew/vcpkg)
find_package(SDL2 REQUIRED)

# Collect all source files
file(GLOB SOURCES "src/*.c")

add_executable(chip8 ${SOURCES})

target_include_directories(chip8 PRIVATE include ${SDL2_INCLUDE_DIRS})
target_link_libraries(chip8 PRIVATE ${SDL2_LIBRARIES})

# Copy ROMs next to the built binary so relative paths in loadROM() work
add_custom_command(TARGET chip8 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/roms $<TARGET_FILE_DIR:chip8>/roms
)
