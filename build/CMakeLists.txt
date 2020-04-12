cmake_minimum_required (VERSION 2.6)
project (sfmaze)

include_directories ("${PROJECT_SOURCE_DIR}/SFML/SFML-2.0-rc_linux/include")
set (EXTRA_LIBS ${EXTRA_LIBS} "${PROJECT_SOURCE_DIR}/SFML/SFML-2.0-rc_linux/lib/libsfml-system.so")
set (EXTRA_LIBS ${EXTRA_LIBS} "${PROJECT_SOURCE_DIR}/SFML/SFML-2.0-rc_linux/lib/libsfml-window.so")

add_executable (sfmaze ../src/maze.cpp)
target_link_libraries (sfmaze  ${EXTRA_LIBS})