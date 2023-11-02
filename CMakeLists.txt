cmake_minimum_required(VERSION 3.20)

project(castle_defense_game)

file(GLOB_RECURSE SOURCE_FILES ${CMAKE_SOURCE_DIR}/src/*.c)
file(GLOB_RECURSE HEADER_FILES ${CMAKE_SOURCE_DIR}/include/*.h)

link_directories(${CMAKE_SOURCE_DIR}/lib)

find_package(OpenGL REQUIRED)

add_compile_definitions("_SOURCE_ROOT=${CMAKE_SOURCE_DIR}")

add_executable(castle_defense_game ${HEADER_FILES} ${SOURCE_FILES})

set_property(TARGET castle_defense_game PROPERTY C_STANDARD 99)

target_include_directories(castle_defense_game PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(castle_defense_game glfw3 OpenGL::GL)
