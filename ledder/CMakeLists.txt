cmake_minimum_required(VERSION 3.17)

project(ledder-emu LANGUAGES CXX)

find_package(SDL2 REQUIRED)
add_executable(ledder-emu emu.cpp)
target_include_directories(
    ledder-emu
    PRIVATE
    ${PROJECT_SOURCE_DIR}/third_party/FixedPoints
    ${PROJECT_SOURCE_DIR}/third_party/sdlpp
)
target_link_libraries(ledder-emu PRIVATE SDL2)