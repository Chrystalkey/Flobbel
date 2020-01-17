cmake_minimum_required(VERSION 3.13)
project(FileServerBackend) #only, I repeat ONLY on linux please

include_directories(${PROJECT_SOURCE_DIR}/../dependencies/sqlite/)

set(FS_SRC main.cpp NetworkRequests.cpp NetworkRequests.h FileManagement.cpp FileManagement.h)

add_executable(FileServerBackend ${FS_SRC})

target_link_libraries(FileServerBackend sqlite)
target_link_libraries(FileServerBackend cryptopp)