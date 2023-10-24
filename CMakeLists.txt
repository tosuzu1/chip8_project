cmake_minimum_required(VERSION 3.14)

project(Chip8)
add_subdirectory(src)
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

ADD_EXECUTABLE(chip8_test1 test/test-01-load.cpp)

target_link_libraries(chip8_test1 PUBLIC chip8)

target_include_directories(chip8_test1 PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}/src"
                          )

ADD_EXECUTABLE(chip8_test2 test/test-02-funcMap.cpp)

target_link_libraries(chip8_test2 PUBLIC chip8)

target_include_directories(chip8_test2 PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}/src"
                          )

ADD_EXECUTABLE(chip8_test3 test/test-03-drawDispBlank.cpp)

target_link_libraries(chip8_test3 PUBLIC chip8 SDL2)

target_include_directories(chip8_test2 PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}/src"
                          )