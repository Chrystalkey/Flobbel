cmake_minimum_required(VERSION 3.14)
project(Flobbel)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 99)
set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++")

add_definitions("-DCURL_STATICLIB")
add_definitions("-DUNICODE")
add_definitions("-D_UNICODE")

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
include_directories(dependencies/curl/include/curl)


link_directories("D:/dev/projects/Flobbel/dependencies/libs" ".")

if(${TEST})
    add_executable(Flobbel testmain.cpp)
elseif(CMAKE_BUILD_TYPE STREQUAL Release)
    set(Flobbel_SRC )
    add_definitions("-O3 -s")
    add_executable(Flobbel WIN32 main.cpp ${Flobbel_SRC})
else()
    set(Flobbel_SRC )
    add_executable(Flobbel main.cpp ${Flobbel_SRC})
endif()


set(LIBS ${LIBS} FlobbelWebStuff)
set(LIBS ${LIBS} FlobbelSafe)
set(LIBS ${LIBS} FlobbelCC)
set(LIBS ${LIBS} FlobbelBackbone)

set(LIBS ${LIBS} libcryptopp.a)
set(LIBS ${LIBS} libcurl.a)
set(LIBS ${LIBS} libnghttp2.a)
set(LIBS ${LIBS} libssh2.a)
set(LIBS ${LIBS} libz.a)
set(LIBS ${LIBS} libssl.a)
set(LIBS ${LIBS} libcrypto.a)
set(LIBS ${LIBS} libbrotlidec-static.a)
set(LIBS ${LIBS} libbrotlienc-static.a)
set(LIBS ${LIBS} libbrotlicommon-static.a)

set(LIBS ${LIBS} Ws2_32)
set(LIBS ${LIBS} Crypt32)
set(LIBS ${LIBS} Wldap32)
set(LIBS ${LIBS} Netapi32)
set(LIBS ${LIBS} shlwapi)
set(LIBS ${LIBS} sqlite)

target_link_libraries(Flobbel ${LIBS})
#target_link_libraries(Flobbel ${LIBS})

