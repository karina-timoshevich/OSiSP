cmake_minimum_required(VERSION 3.28)
project(LabRab5_Timoshevich_OSiSP)

set(CMAKE_CXX_STANDARD 17)

add_executable(server server.cpp)
target_link_libraries(server PRIVATE  ws2_32)

add_executable(client client.cpp)
target_link_libraries(client PRIVATE ws2_32)
