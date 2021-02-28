set(TARGET SoftHeapCpp)

add_library(${TARGET} INTERFACE )

target_include_directories(${TARGET}
INTERFACE
    .
)

if (MSVC)
    add_custom_target(${TARGET}_ SOURCES SoftHeapCpp.h Utils.h)
    SET_TARGET_PROPERTIES (${TARGET}_ PROPERTIES FOLDER SoftHeapCpp)
endif()


add_subdirectory(Test)