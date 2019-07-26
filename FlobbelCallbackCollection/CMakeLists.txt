project(FlobbelCallbackCollection)

include_directories(../FlobbelBackbone)

set(CC_SRC flobcallbackcollection.h flobcallbackcollection.cpp)

add_library(FlobbelCC STATIC ${CC_SRC})