set(TARGET SoftHeapCppTest)

add_executable(${TARGET} 
    SoftHeapCppTest.cpp
)

target_link_libraries(${TARGET} SoftHeapCpp gtest gtest_main Common)
SET_TARGET_PROPERTIES (${TARGET} PROPERTIES FOLDER SoftHeapCpp)