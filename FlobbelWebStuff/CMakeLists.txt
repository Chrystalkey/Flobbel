project(FlobbelWebStuff)

include_directories(../FlobbelBackbone)

add_library(FlobbelWebStuff STATIC FlobWS.cpp FlobWS.h)