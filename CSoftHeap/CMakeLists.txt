set(TARGET CSoftHeap)

add_library(${TARGET} 
    CSoftHeap.h
    CSoftHeap.cpp
)

target_include_directories(${TARGET} PUBLIC .)

SET_TARGET_PROPERTIES (${TARGET} PROPERTIES FOLDER CSoftHeap)

add_subdirectory(Test)