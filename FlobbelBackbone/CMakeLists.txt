project(FlobbelBackbone)

add_library(FlobbelBackbone global_functions.cpp)

target_link_libraries(FlobbelBackbone Iphlpapi)