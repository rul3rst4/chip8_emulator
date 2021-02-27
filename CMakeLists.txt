# Base Project for CMAKE
cmake_minimum_required(VERSION 3.15)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(main)

set(CMAKE_CXX_STANDARD 17)

#include_directories("./implot")

file(GLOB resources "./res")
file(COPY ${resources} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_executable(main main.cpp)
target_link_libraries(main)
