cmake_minimum_required(VERSION 3.15)
project(Kendinsky)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenMP REQUIRED)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Release" "Debug")
endif()

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -fopenmp -pthread")
set(CMAKE_CXX_FLAGS_DEBUG  "-O0 -g2 -ggdb -fopenmp -pthread")

include_directories(${CMAKE_CURRENT_LIST_DIR}/framework)
include_directories(${CMAKE_CURRENT_LIST_DIR}/mining)

add_subdirectory(framework)
add_subdirectory(mining)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/../bin)

# add_executable(io_test test/io_test.cpp)
# target_link_libraries(io_test pthread framework)
# add_executable(trie_test test/trie_test.cpp)
# target_link_libraries(trie_test framework)
# add_executable(perf_test test/perf_test.cpp)
# target_link_libraries(perf_test framework)
# add_executable(run_test test/run_test.cpp)
# target_link_libraries(run_test framework)

add_executable(mining_test test/mining_test.cpp)
target_link_libraries(mining_test mining framework)

add_executable(pre pre_src/pre.cpp)
target_link_libraries(pre framework)

add_executable(main main.cpp)
target_link_libraries(main framework)