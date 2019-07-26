project(FlobbelSafe)

include_directories(../FlobbelBackbone/)
include_directories(../cryptopp/include)
include_directories(../sqlite/)

add_library(FlobbelSafe FlobbelSafe.cpp FlobbelSafe.h)

if(CMAKE_BUILD_TYPE STREQUAL DEBUG)
    target_link_libraries(FlobbelSafe D:/dev/cryptopp820/library/libcryptopp64_dbg.a)
else()
    target_link_libraries(FlobbelSafe D:/dev/cryptopp820/library/libcryptopp64_rel.a)
endif()