cmake_minimum_required(VERSION 3.10.0)
project(mosh)

set(EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(C_STANDARD 23)
set(CMAKE_C_STANDARD 23)
set(C_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Debug)
# set(CMAKE_EXE_LINKER_FLAGS "-static")

add_compile_options(-Wall -Wextra -g)

# Adding libraries
find_package(PkgConfig REQUIRED)
pkg_search_module(READLINE REQUIRED readline)

# For static linking
set(READLINE_LIBRARIES ${READLINE_STATIC_LIBRARIES})

# Compiler options
file(GLOB_RECURSE SOURCES PUBLIC src/*.c)
add_executable(${CMAKE_PROJECT_NAME} main.c ${SOURCES})
target_compile_options(${CMAKE_PROJECT_NAME} PUBLIC -fsanitize=address)
target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC include/ ${READLINE_INCLUDE_DIRS})

# Linking
target_link_options(${CMAKE_PROJECT_NAME} PUBLIC -fsanitize=address)
target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC
    ${READLINE_LIBRARIES}
)
