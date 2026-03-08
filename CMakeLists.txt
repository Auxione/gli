cmake_minimum_required(VERSION 3.31)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(glad)

project(gli)

add_library(gli STATIC)

target_include_directories(
        gli
        PUBLIC include
)

target_link_libraries(
        gli
        PUBLIC glad
)

target_sources(
        gli
        PUBLIC src/gli.cpp
)