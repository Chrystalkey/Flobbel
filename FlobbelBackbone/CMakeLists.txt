project(FlobbelBackbone)

add_library(FlobbelBackbone STATIC global_functions.cpp)

target_link_libraries(FlobbelBackbone Iphlpapi)