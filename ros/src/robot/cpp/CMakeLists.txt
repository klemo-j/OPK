cmake_minimum_required(VERSION 3.10)
project(zadanie1)

set(CMAKE_CXX_STANDARD 20)
find_package(OpenCV REQUIRED)
find_package(yaml-cpp REQUIRED)

add_library(robot_lib STATIC
        src/Robot.cpp
        src/Environment.cpp
        src/Geometry.cpp
        src/Lidar.cpp
        src/GameEnvironment.cpp
        src/GameLogic.cpp
)
target_include_directories(robot_lib PUBLIC include ${OpenCV_INCLUDE_DIRS})
target_link_libraries(robot_lib ${OpenCV_LIBS} yaml-cpp)


install(TARGETS robot_lib
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
)

# Inštalácia hlavičkových súborov, aby ich iné balíky (ROS) videli
install(DIRECTORY include/
        DESTINATION include
)


find_package(GTest REQUIRED)

add_executable(lidar_benchmark Test/lidar_benchmark.cpp)
target_include_directories(lidar_benchmark PRIVATE include)
target_link_libraries(lidar_benchmark robot_lib GTest::GTest GTest::Main)

# Toto nemazat, v pripade ak to zmazete, github classroom nebude fungovat
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/hidden_tests/CMakeLists.txt)
    add_subdirectory(hidden_tests)
endif()