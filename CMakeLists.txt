cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED On)

project(
    beamburst2
    LANGUAGES CXX
)

add_compile_options(
    -g
    -O3
    -Wall
    -Werror
    -Wextra
)

find_package(Git REQUIRED)
find_package(PNG REQUIRED)

add_executable(bb2 src/beamburst2.cpp)
target_link_libraries(bb2 png)

