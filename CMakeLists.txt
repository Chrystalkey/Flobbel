cmake_minimum_required(VERSION 3.14)
project(Flobbel)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(FlobbelBackbone)
add_subdirectory(FlobbelCallbackCollection)
add_subdirectory(FlobbelSafe)
add_subdirectory(sqlite)

include_directories(FlobbelBackbone)
include_directories(FlobbelCallbackCollection)
include_directories(FlobbelSafe)
include_directories(cryptopp/include)
include_directories(sqlite)

add_definitions("-DUNICODE")
add_definitions("-D_UNICODE")
add_definitions("-DSQL_AVAILABLE")

if(${TEST})
    add_executable(Flobbel testmain.cpp)
elseif(CMAKE_BUILD_TYPE STREQUAL Release)
    set(Flobbel_SRC )
    add_executable(Flobbel WIN32 main.cpp)
else()
    set(Flobbel_SRC )
    add_executable(Flobbel main.cpp)
endif()


target_link_libraries(Flobbel FlobbelSafe)
target_link_libraries(Flobbel FlobbelCC)
target_link_libraries(Flobbel FlobbelBackbone)
target_link_libraries(Flobbel shlwapi)
target_link_libraries(Flobbel Netapi32)
target_link_libraries(Flobbel sqlite)

set(CMAKE_CXX_FLAGS "-static -static-libgcc -static-libstdc++")