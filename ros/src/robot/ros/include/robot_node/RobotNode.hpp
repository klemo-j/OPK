#pragma once

#include <memory>
#include <atomic>

#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/pose2_d.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <nav_msgs/msg/path.hpp>
#include <tf2_ros/transform_broadcaster.h>

#include <robot_msgs/msg/game_state.hpp>

#include "robot/Robot.hpp"
#include "environment/GameEnvironment.hpp"

class RobotNode : public rclcpp::Node {
public:
    RobotNode();

private:
    void cmdVelCallback(const geometry_msgs::msg::Twist::SharedPtr msg);
    void publishStateTimer();
    void gameStateCallback(const robot_msgs::msg::GameState::SharedPtr msg);

    std::shared_ptr<environment::GameEnvironment> game_env_;
    std::unique_ptr<robot::Robot>                 robot_;

    rclcpp::Publisher<geometry_msgs::msg::Pose2D>::SharedPtr      pose_pub_;
    rclcpp::Publisher<geometry_msgs::msg::PoseStamped>::SharedPtr pose_stamped_pub_;
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr       twist_pub_;
    rclcpp::Publisher<nav_msgs::msg::Path>::SharedPtr             path_pub_;
    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr    cmd_sub_;
    rclcpp::Subscription<robot_msgs::msg::GameState>::SharedPtr   game_state_sub_;
    rclcpp::TimerBase::SharedPtr                                  state_timer_;

    std::unique_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;

    nav_msgs::msg::Path path_;

    std::string robot_frame_;
    std::string world_frame_;

    std::atomic<bool> game_over_{false};
};