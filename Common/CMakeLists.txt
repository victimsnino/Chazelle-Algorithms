set(TARGET Common)

add_library(${TARGET} INTERFACE )

target_include_directories(${TARGET}
INTERFACE
    .
)

if (MSVC)
    add_custom_target(${TARGET}_ SOURCES Common.h)
endif()
