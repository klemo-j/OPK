cmake_minimum_required(VERSION 4.1)
project(zadanie1)

set(CMAKE_CXX_STANDARD 20)

add_executable(zadanie1 main.cpp
        src/Robot.cpp
        src/Environment.cpp
        src/Geometry.cpp
        src/Lidar.cpp)
