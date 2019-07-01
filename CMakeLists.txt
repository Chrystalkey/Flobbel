cmake_minimum_required(VERSION 3.14)
project(Flobbel)
set(CMAKE_CXX_STANDARD 14)

add_subdirectory(FlobbelCallbackCollection)
# add_subdirectory(FlobbelSaveLib)

set(Flobbel_SRC main.cpp)

add_executable(Flobbel main.cpp global_functions.h "global_constants.h")

target_link_libraries(Flobbel FlobbelCC)
target_link_libraries(Flobbel Iphlpapi)
# target_link_libraries(Flobbel FlobbelSL)