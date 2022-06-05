cmake_minimum_required(VERSION 3.22)
project(.)

set(CMAKE_CXX_STANDARD 14)

include_directories(src)

add_executable(ModuleMining
        src/CanonibalLabel.cpp
        src/CanonicalLabel.h
        src/CLMap.cpp
        src/CLMap.h
        src/EdgeX.cpp
        src/EdgeX.h
        src/GraMiCounter.cpp
        src/GraMiCounter.h
        src/GraphX.cpp
        src/GraphX.h
        src/Miner.cpp
        src/Miner.h
        src/MyMiner.cpp
        src/MyMiner.h
        src/NodeX.cpp
        src/NodeX.h
        src/Pattern.cpp
        src/Pattern.h
        src/QueueSimulator.cpp
        src/QueueSimulator.h
        src/Settings.h
        src/utils.cpp
        src/utils.h
        main.cpp)
