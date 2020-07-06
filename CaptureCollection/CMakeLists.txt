project(CaptureCollection)

include_directories(../CaptureTypes)

set(CC_SRC capturecollection.h
        capturecollection.cpp
        info_type.h
        info_type.cpp
        process_type.h
        process_type.cpp
        keypress_type.h
        keypress_type.cpp
        screentime_type.cpp
        screentime_type.cpp
        )

add_library(CaptureCollection STATIC ${CC_SRC})