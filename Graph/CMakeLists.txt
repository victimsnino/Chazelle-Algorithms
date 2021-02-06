set(TARGET Graph)

add_library(${TARGET} 
    Graph.h
    Graph.cpp
)

target_include_directories(${TARGET} PUBLIC .)
SET_TARGET_PROPERTIES (${TARGET} PROPERTIES FOLDER Graph)

add_subdirectory(Test)