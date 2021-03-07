set(TARGET Common)

add_library(${TARGET} INTERFACE )

target_include_directories(${TARGET}
INTERFACE
    .
)

target_link_libraries(${TARGET} INTERFACE gcem)

if (MSVC)
    add_custom_target(${TARGET}_ SOURCES Common.h)
endif()
