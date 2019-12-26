project(FlobbelWebStuff)
cmake_minimum_required(VERSION 3.14)

include_directories(../FlobbelBackbone/)
include_directories(../dependencies/)

add_library(FlobbelWebStuff STATIC FlobWS.cpp FlobWS.h)


set(LIBDIR "D:/developement/msys2_64/mingw64/lib")

set(WSLIBS ${WSLIBS} ${PROJECT_SOURCE_DIR}/../dependencies/curl/lib/libcurl.a)
set(WSLIBS ${WSLIBS} ${LIBDIR}/libidn2.a)
set(WSLIBS ${WSLIBS} ${LIBDIR}/libpsl.a)
set(WSLIBS ${WSLIBS} ${LIBDIR}/libssl.a)
set(WSLIBS ${WSLIBS} ${LIBDIR}/libcrypto.a)
set(WSLIBS ${WSLIBS} ${LIBDIR}/libssl.a)
set(WSLIBS ${WSLIBS} ${LIBDIR}/libcrypto.a)
set(WSLIBS ${WSLIBS} ${LIBDIR}/libiconv.a)
set(WSLIBS ${WSLIBS} Netapi32)
set(WSLIBS ${WSLIBS} Ws2_32)
set(WSLIBS ${WSLIBS} ${LIBDIR}/libunistring.dll.a)

target_link_libraries(FlobbelWebStuff ${WSLIBS})