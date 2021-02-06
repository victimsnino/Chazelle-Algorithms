set(TARGET MST)

add_library(${TARGET} 
    MST.h
    MST.cpp
)

target_include_directories(${TARGET} PUBLIC .)
target_link_libraries(${TARGET}
    PRIVATE
        SoftHeapCpp
    PUBLIC
        Graph
)

SET_TARGET_PROPERTIES (${TARGET} PROPERTIES FOLDER MST)
