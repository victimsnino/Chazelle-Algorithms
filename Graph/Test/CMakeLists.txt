set(TARGET GraphTest)

add_executable(${TARGET} 
    GraphTest.cpp
)

target_link_libraries(${TARGET} Graph gtest gtest_main Common)
SET_TARGET_PROPERTIES (${TARGET} PROPERTIES FOLDER Graph)

add_test(${TARGET} ${TARGET})