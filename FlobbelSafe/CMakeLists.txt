project(FlobbelSafe)

include_directories(../FlobbelBackbone/)
include_directories(../dependencies/sqlite/)

add_library(FlobbelSafe STATIC FlobbelSafe.cpp FlobbelSafe.h)