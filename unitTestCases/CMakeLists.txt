#cmake_minimum_required(VERSION 3.0 FATAL_ERROR)


## Add Google Test Framework
set(EXT_PROJECTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/googleFiles/googletest)
add_subdirectory("${EXT_PROJECTS_DIR}")

#~ ## Add Google Mock Framework
#~ set(EXT_PROJECTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext)
#~ add_subdirectory("${EXT_PROJECTS_DIR}/gmock")

#MESSAGE("---------------->${gtest_SOURCE_DIR}")
#MESSAGE("---------------->${gmock_SOURCE_DIR}")
#include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
#include_directories(${gmock_SOURCE_DIR}/include ${gmock_SOURCE_DIR})

## Add Include Directories
#include_directories( ${ALLJOYNLIBS_INCLUDE_DIRS})


## Add Include Directories
include_directories(${EXT_PROJECTS_DIR}/include)

