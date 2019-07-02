cmake_minimum_required(VERSION 3.14)
project(Flobbel)
set(CMAKE_CXX_STANDARD 14)

add_subdirectory(FlobbelBackbone)
add_subdirectory(FlobbelCallbackCollection)
add_subdirectory(FlobbelSafe)

include_directories(FlobbelBackbone)
include_directories(FlobbelCallbackCollection)
include_directories(FlobbelSafe)

if(${TEST})
    add_executable(Flobbel testmain.cpp)
else()
    set(Flobbel_SRC )
    add_executable(Flobbel main.cpp)
endif()



target_link_libraries(Flobbel FlobbelSafe)
target_link_libraries(Flobbel FlobbelCC)
target_link_libraries(Flobbel FlobbelBackbone)

set(CMAKE_CXX_FLAGS "-static -static-libgcc -static-libstdc++")