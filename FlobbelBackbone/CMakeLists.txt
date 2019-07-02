cmake_minimum_required(VERSION 3.14)
project(FlobbelBackbone)
set(CMAKE_CXX_STANDARD 14)

add_library(FlobbelBackbone global_functions.cpp)

target_link_libraries(FlobbelBackbone Iphlpapi)