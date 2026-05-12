//
// Created by kllemo on 4/8/26.
//
#include <rclcpp/rclcpp.hpp>

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<rclcpp::Node>("robot_node");
    RCLCPP_INFO(node->get_logger(), "Robot node started.");
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}