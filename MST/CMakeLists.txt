set(TARGET MST)

add_library(${TARGET} 
    MST.h
    MST.cpp

    MSTTree.cpp
    MSTTree.h

    MSTTreeBuilder.cpp
    MSTTreeBuilder.h

    MSTTreeSubgraph.cpp
    MSTTreeSubgraph.h

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
        spdlog::spdlog_header_only
)

SET_TARGET_PROPERTIES (${TARGET} PROPERTIES FOLDER MST)

add_subdirectory(Test)