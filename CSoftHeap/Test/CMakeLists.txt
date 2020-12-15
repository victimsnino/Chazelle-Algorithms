set(TARGET CSoftHeapTest)

add_executable(${TARGET} 
    CSoftHeapTest.cpp
)

target_link_libraries(${TARGET} CSoftHeap gtest gtest_main)
SET_TARGET_PROPERTIES (${TARGET} PROPERTIES FOLDER CSoftHeap)