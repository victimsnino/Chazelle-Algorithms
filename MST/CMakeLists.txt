set(TARGET MST)

add_library(${TARGET} 
    MST.h
    MST.cpp

    MSTTree.cpp
    MSTTree.h

    MSTStack.cpp
    MSTStack.h

    MSTStackSubgraph.cpp
    MSTStackSubgraph.h

    MSTUtils.h
    MSTUtils.cpp

    MSTSoftHeapDecorator.h
    MSTSoftHeapDecorator.cpp
)

target_include_directories(${TARGET} PUBLIC .)
target_link_libraries(${TARGET}
    PUBLIC
        SoftHeapCpp
        Common
        Graph
)

SET_TARGET_PROPERTIES (${TARGET} PROPERTIES FOLDER MST)

add_subdirectory(Test)