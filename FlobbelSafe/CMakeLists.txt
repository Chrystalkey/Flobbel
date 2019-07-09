cmake_minimum_required(VERSION 3.14)

project(FlobbelSafe)

set(CMAKE_CXX_STANDARD 17)

include_directories(../FlobbelBackbone/)
include_directories(../cryptopp/include)

add_library(FlobbelSafe FlobbelSafe.cpp FlobbelSafe.h)

if(CMAKE_BUILD_TYPE STREQUAL DEBUG)
    target_link_libraries(FlobbelSafe D:/dev/cryptopp820/library/libcryptopp64_dbg.a)
else()
    target_link_libraries(FlobbelSafe D:/dev/cryptopp820/library/libcryptopp64_rel.a)
endif()