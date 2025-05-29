cmake_minimum_required(VERSION 3.10.0)
project(jush)

set(EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(C_STANDARD 23)
set(CMAKE_C_STANDARD 23)
set(C_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCES PUBLIC src/*.c)

add_executable(${CMAKE_PROJECT_NAME} main.c ${SOURCES})

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC include/)
add_compile_options(-Wall -Wextra)

target_compile_options(${CMAKE_PROJECT_NAME} PUBLIC -fsanitize=address)
target_link_options(${CMAKE_PROJECT_NAME} PUBLIC -fsanitize=address)
