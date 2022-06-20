cmake_minimum_required(VERSION 3.15)
project(mining)

set(CMAKE_CXX_STANDARD 17)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Release" "Debug")
endif()

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG  "-O0 -g2 -ggdb")

file(GLOB MINING_SRC_LIST *)
# message(STATUS "MINING_SRC_LIST=${MINING_SRC_LIST}")

add_library(mining ${MINING_SRC_LIST} core_file.h)
set_target_properties(mining PROPERTIES LINKER_LANGUAGE CXX)