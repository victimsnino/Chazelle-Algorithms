set(TARGET SoftHeapC)

add_library(${TARGET} 
    SoftHeapC.h
    SoftHeapC.cpp
)

target_include_directories(${TARGET} PUBLIC .)

SET_TARGET_PROPERTIES (${TARGET} PROPERTIES FOLDER SoftHeapC)

add_subdirectory(Test)