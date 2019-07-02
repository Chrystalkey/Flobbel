project(FlobbelCallbackCollection)
cmake_minimum_required(VERSION 3.14)

include_directories(../FlobbelBackbone)

set(CC_SRC flobcallbackcollection.h flobcallbackcollection.cpp)

add_library(FlobbelCC STATIC ${CC_SRC})