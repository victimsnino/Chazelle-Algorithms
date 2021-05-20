set(TARGET Graph)

add_library(${TARGET} 
    Graph.h
    Graph.cpp

    GraphDetails.h
    GraphDetails.cpp

    Kruskal.cpp
    Kruskal.h
) 

target_include_directories(${TARGET} PUBLIC .)
target_link_libraries(${TARGET} PRIVATE Common)
SET_TARGET_PROPERTIES (${TARGET} PROPERTIES FOLDER Graph)

add_subdirectory(Test)