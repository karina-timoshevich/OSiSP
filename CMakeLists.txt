cmake_minimum_required(VERSION 3.28)
project(OSiSP)

set(CMAKE_CXX_STANDARD 17)

add_executable(OSiSP main.cpp)
target_link_libraries(OSiSP pdh)
