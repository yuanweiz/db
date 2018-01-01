cmake_minimum_required(VERSION 2.6)

add_library (db Pager.cpp PagePtr.cpp Exception.cpp)

include_directories(${PROJECT_SOURCE_DIR})
set(CXX_FLAGS
 -g
 -D_FILE_OFFSET_BITS=64
 -Wall
 -Wextra
 -Werror
 -Wconversion
 -Wno-unused-parameter
 -Wold-style-cast
 -Woverloaded-virtual
 -Wpointer-arith
 -Wshadow
 -Wwrite-strings
 -march=native
 -rdynamic
 )

string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -finline-limit=1000 -DNDEBUG")

message(STATUS "CXX_FLAGS = " ${CMAKE_CXX_FLAGS} " " ${CMAKE_CXX_FLAGS_${BUILD_TYPE}})

enable_testing()
find_package(GTest )
find_package(Threads)
if (GTEST_FOUND)
    include_directories(${GTEST_INCLUDE_DIRS})
    add_subdirectory(test)
endif()
