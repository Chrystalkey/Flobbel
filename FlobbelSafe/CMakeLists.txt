project(FlobbelSafe)

include_directories(../FlobbelBackbone/)
include_directories(../dependencies/sqlite/)

set(ENCRYPTION_SRC rawdataprocessing.cpp rawdataprocessing.h FileServerBackend/main.cpp)

add_library(FlobbelSafe STATIC FlobbelSafe.cpp FlobbelSafe.h ${ENCRYPTION_SRC})


target_link_libraries(FlobbelSafe ${LIBDIR}/libcryptopp.a)
target_link_libraries(FlobbelSafe sqlite)