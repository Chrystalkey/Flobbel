cmake_minimum_required(VERSION 3.14)

project(FlobbelSave)

set(CMAKE_CXX_STANDARD 14)

include_directories(../FlobbelBackbone)


add_library(FlobbelSafe FlobbelSafe.cpp)
