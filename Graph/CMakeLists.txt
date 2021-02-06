set(TARGET Graph)

add_library(${TARGET} 
    Graph.h
    Graph.cpp
)

target_include_directories(${TARGET} PUBLIC .)