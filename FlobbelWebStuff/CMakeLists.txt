project(FlobbelWebStuff)

include_directories(../FlobbelBackbone/)
include_directories(../dependencies/)

add_library(FlobbelWebStuff STATIC FlobWS.cpp FlobWS.h)