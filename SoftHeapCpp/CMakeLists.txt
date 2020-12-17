set(TARGET SoftHeapCpp)

add_library(${TARGET} 
    SoftHeapCpp.h
    SoftHeapCpp.cpp

    Utils.h
)

target_include_directories(${TARGET} PUBLIC .)

SET_TARGET_PROPERTIES (${TARGET} PROPERTIES FOLDER SoftHeapCpp)

add_subdirectory(Test)