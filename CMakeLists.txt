cmake_minimum_required(VERSION 3.14)
project(Flobbel)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 99)

add_subdirectory(dependencies/sqlite)
add_subdirectory(FlobbelBackbone)
add_subdirectory(FlobbelCallbackCollection)
add_subdirectory(FlobbelSafe)
add_subdirectory(FlobbelWebStuff)

include_directories(FlobbelBackbone)
include_directories(FlobbelCallbackCollection)
include_directories(FlobbelSafe)
include_directories(FlobbelWebStuff)
include_directories(dependencies/cryptopp/include)
include_directories(dependencies/sqlite)

add_definitions("-DUNICODE")
add_definitions("-D_UNICODE")

#CURL STUFF

#set(BUILD_CURL_EXE OFF CACHE BOOL "")
#set(CURL_STATICLIB ON CACHE BOOL "")
#set(CURL_ZLIB OFF CACHE BOOL "")

#add_subdirectory(dependencies/curl-7.65.3)

if(${TEST})
    add_executable(Flobbel testmain.cpp)
elseif(CMAKE_BUILD_TYPE STREQUAL Release)
    set(Flobbel_SRC )
    add_executable(Flobbel WIN32 main.cpp)
else()
    set(Flobbel_SRC )
    add_executable(Flobbel main.cpp)
endif()

target_link_libraries(Flobbel FlobbelBackbone)
target_link_libraries(Flobbel FlobbelWebStuff)
target_link_libraries(Flobbel FlobbelSafe)
target_link_libraries(Flobbel FlobbelCC)
target_link_libraries(Flobbel shlwapi)
target_link_libraries(Flobbel Netapi32)
target_link_libraries(Flobbel sqlite)
if(CMAKE_BUILD_TYPE STREQUAL DEBUG)
    target_link_libraries(FlobbelSafe D:/dev/cryptopp820/library/libcryptopp64_dbg.a)
else()
    target_link_libraries(FlobbelSafe D:/dev/cryptopp820/library/libcryptopp64_rel.a)
endif()

set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++")