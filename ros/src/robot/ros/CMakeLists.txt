cmake_minimum_required(VERSION 3.8)
project(robot)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ── ROS dependencie ──────────────────────────────────────────────────────────
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(std_srvs REQUIRED)
find_package(visualization_msgs REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(robot_msgs REQUIRED)

# ── Pripoj cpp/ podadresár (čistá C++ logika) ────────────────────────────────
add_subdirectory(../cpp ${CMAKE_CURRENT_BINARY_DIR}/cpp_build)

# Spoločné dependencie pre všetky executable
set(NODE_DEPS
        rclcpp
        geometry_msgs
        nav_msgs
        sensor_msgs
        std_msgs
        std_srvs
        visualization_msgs
        tf2
        tf2_ros
        tf2_geometry_msgs
        robot_msgs
)

# ── robot_node ───────────────────────────────────────────────────────────────
add_executable(robot_node src/robot_node.cpp)
target_include_directories(robot_node PRIVATE include)
ament_target_dependencies(robot_node ${NODE_DEPS})
target_link_libraries(robot_node robot_lib)

# ── lidar_node ───────────────────────────────────────────────────────────────
add_executable(lidar_node src/lidar_node.cpp)
target_include_directories(lidar_node PRIVATE include)
ament_target_dependencies(lidar_node ${NODE_DEPS})
target_link_libraries(lidar_node robot_lib)

# ── map_node ─────────────────────────────────────────────────────────────────
add_executable(map_node src/map_node.cpp)
target_include_directories(map_node PRIVATE include)
ament_target_dependencies(map_node ${NODE_DEPS})
target_link_libraries(map_node robot_lib)

# ── game_node ────────────────────────────────────────────────────────────────
add_executable(game_node src/game_node.cpp)
target_include_directories(game_node PRIVATE include)
ament_target_dependencies(game_node ${NODE_DEPS})
target_link_libraries(game_node robot_lib)

# ── Inštalácia binárok ───────────────────────────────────────────────────────
install(TARGETS
        robot_node
        lidar_node
        map_node
        game_node
        DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY ../cpp/config/
        DESTINATION share/${PROJECT_NAME}/config
)
install(DIRECTORY launch/
        DESTINATION share/${PROJECT_NAME}/launch
        OPTIONAL
)
install(DIRECTORY rviz/
        DESTINATION share/${PROJECT_NAME}/rviz
        OPTIONAL
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()