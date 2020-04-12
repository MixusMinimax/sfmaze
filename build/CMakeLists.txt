cmake_minimum_required (VERSION 3.1)
project (sfmaze)

find_package(SFML 2.5 COMPONENTS system window graphics audio REQUIRED)

include_directories ("${PROJECT_SOURCE_DIR}/SFML/SFML-2.5-rc_linux/include")

add_definitions ("-std=c++17")
add_executable (sfmaze ../src/maze.cpp)

target_link_libraries (sfmaze sfml-graphics)