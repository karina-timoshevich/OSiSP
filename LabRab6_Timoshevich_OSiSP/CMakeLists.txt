cmake_minimum_required(VERSION 3.28)
project(LabRab6_Timoshevich_OSiSP)

set(CMAKE_CXX_STANDARD 17)

add_executable(LabRab6_Timoshevich_OSiSP main.cpp)

target_link_libraries(LabRab6_Timoshevich_OSiSP wbemuuid iphlpapi wlanapi)
