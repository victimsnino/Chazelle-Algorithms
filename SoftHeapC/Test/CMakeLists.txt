set(TARGET SoftHeapCTest)

add_executable(${TARGET} 
    SoftHeapCTest.cpp
)

target_link_libraries(${TARGET} SoftHeapC gtest gtest_main Common)
SET_TARGET_PROPERTIES (${TARGET} PROPERTIES FOLDER SoftHeapC)

add_test(${TARGET} ${TARGET})