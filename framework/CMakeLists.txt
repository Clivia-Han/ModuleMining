cmake_minimum_required(VERSION 3.15)
project(framework)

set(CMAKE_CXX_STANDARD 17)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Release" "Debug")
endif()

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG  "-O0 -g2 -ggdb")

file(GLOB FRAM_SRC_LIST *)
# message(STATUS "FRAM_SRC_LIST=${FRAM_SRC_LIST}")

add_library(framework ${FRAM_SRC_LIST})
set_target_properties(framework PROPERTIES LINKER_LANGUAGE CXX)