cmake_minimum_required(VERSION 3.16...3.28)

project(BeamBurst2 VERSION 0.0.1 LANGUAGES CXX C)

find_library(BB2_PNG NAMES png libpng)
if (DEFINED BB2_PNG)
    message(STATUS "Found libpng")
endif()

# main executable
add_executable(bb2)

target_sources(bb2 PRIVATE src/main.cpp) 

target_include_directories(bb2 PRIVATE src arrayalgebra)

target_link_libraries(bb2 PRIVATE png)

