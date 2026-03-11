cmake_minimum_required(VERSION 4.1)
project(CV2)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenCV REQUIRED)
target_link_libraries( CV2 PRIVATE ${OpenCV_LIBS}) # Linkovať OpenCV


add_executable(CV2 main.cpp)