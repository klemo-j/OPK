cmake_minimum_required(VERSION 4.1)
project(CV2)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenCV REQUIRED)


add_executable(CV2
        src/main.cpp
        lib/Shape.cpp
        lib/Canvas.cpp
        lib/include/Canvas.hpp
)


target_include_directories(CV2 PRIVATE lib/include)

target_link_libraries(CV2 PUBLIC ${OpenCV_LIBS})

#target_compile_options(CV2 PUBLIC -Wall -Wextra -Wconversion -Wshadow -Wpedantic -Werror)