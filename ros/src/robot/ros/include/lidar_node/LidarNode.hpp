#pragma once

#include <memory>
#include <mutex>
#include <string>

#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/pose2_d.hpp>
#include <sensor_msgs/msg/laser_scan.hpp>
#include <tf2_ros/static_transform_broadcaster.h>

#include "environment/GameEnvironment.hpp"
#include "environment/Lidar.hpp"
#include "types/Geometry.hpp"

class LidarNode : public rclcpp::Node {
public:
    LidarNode();

private:
    void poseCallback(const geometry_msgs::msg::Pose2D::SharedPtr msg);
    void scanTimer();

    // Čistá logika
    std::shared_ptr<environment::GameEnvironment> game_env_;
    std::unique_ptr<lidar::Lidar>                 lidar_;
    lidar::Config                                 lidar_cfg_;

    // ROS
    rclcpp::Publisher<sensor_msgs::msg::LaserScan>::SharedPtr scan_pub_;
    rclcpp::Subscription<geometry_msgs::msg::Pose2D>::SharedPtr pose_sub_;
    rclcpp::TimerBase::SharedPtr scan_timer_;
    std::unique_ptr<tf2_ros::StaticTransformBroadcaster> static_tf_;

    // Stav
    std::mutex state_mutex_;
    geometry::RobotState last_state_{0.0, 0.0, 0.0, {0.0, 0.0}};

    std::string laser_frame_;
    std::string robot_frame_;
};