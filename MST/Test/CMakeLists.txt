set(TARGET MSTTest)

add_executable(${TARGET} 
    MSTTest.cpp
)

target_link_libraries(${TARGET} MST gtest gtest_main gmock_main Common)
SET_TARGET_PROPERTIES (${TARGET} PROPERTIES FOLDER MST)

add_test(${TARGET} ${TARGET})