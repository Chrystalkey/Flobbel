cmake_minimum_required(VERSION 3.13)
project(Flobbel)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 99)
set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++")

if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
    set(LIBDIR "D:/development/msys2_64/mingw32/lib")
else()
    set(LIBDIR "D:/development/msys2_64/mingw64/lib")
endif()

add_definitions("-DCURL_STATICLIB")
add_definitions("-DUNICODE")
add_definitions("-D_UNICODE")
add_definitions("-ggdb")
add_definitions("-g")

include_directories(CaptureCollection)
include_directories(dependencies/)

add_subdirectory(dependencies/sqlite)
add_subdirectory(CaptureCollection)

if(${TEST})
    add_executable(Flobbel testmain.cpp)
elseif(CMAKE_BUILD_TYPE STREQUAL Release)
    add_definitions("-O3 -s")
    add_executable(Flobbel WIN32 main.cpp global_functions.cpp FlobbelSafe.cpp FlobbelSafe.h flob_exceptions.h logging.cpp logging.h)
else()
    add_executable(Flobbel main.cpp global_functions.cpp FlobbelSafe.cpp FlobbelSafe.h flob_exceptions.h logging.cpp logging.h)
endif()

set(LIBS ${LIBS} CaptureCollection)
set(LIBS ${LIBS} sqlite)

set(LIBS ${LIBS} gdi32)
set(LIBS ${LIBS} shlwapi)
set(LIBS ${LIBS} iphlpapi)
set(LIBS ${LIBS} Netapi32)

target_link_libraries(Flobbel ${LIBS})