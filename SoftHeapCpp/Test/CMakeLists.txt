set(TARGET SoftHeapCppTest)

add_executable(${TARGET} 
    SoftHeapCppTest.cpp
)

target_link_libraries(${TARGET} SoftHeapCpp SoftHeapC gtest gtest_main gmock Common)
SET_TARGET_PROPERTIES (${TARGET} PROPERTIES FOLDER SoftHeapCpp)

add_test(${TARGET} ${TARGET})