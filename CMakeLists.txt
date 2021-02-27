cmake_minimum_required(VERSION 3.10)

project(Chazelle-Algorithms LANGUAGES CXX)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(GRAPHVIZ_DISABLE "Force disable graphviz" OFF)

if (GRAPHVIZ_DISABLE)
  ADD_DEFINITIONS(-DGRAPHVIZ_DISABLED=1)
endif()

set(CompilerFlags
        CMAKE_CXX_FLAGS
        CMAKE_CXX_FLAGS_DEBUG
        CMAKE_CXX_FLAGS_RELEASE
        CMAKE_C_FLAGS
        CMAKE_C_FLAGS_DEBUG
        CMAKE_C_FLAGS_RELEASE
        )
foreach(CompilerFlag ${CompilerFlags})
  string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
endforeach()

add_subdirectory(googletest EXCLUDE_FROM_ALL)
SET_TARGET_PROPERTIES (gtest PROPERTIES FOLDER gtest/)
SET_TARGET_PROPERTIES (gtest_main PROPERTIES FOLDER gtest/)

enable_testing()

add_subdirectory(Common)
add_subdirectory(MST)
add_subdirectory(Graph)
add_subdirectory(SoftHeapC)
add_subdirectory(SoftHeapCpp)