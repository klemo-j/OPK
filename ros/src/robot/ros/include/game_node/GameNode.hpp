#pragma once

#include <memory>
#include <mutex>
#include <string>

#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/pose2_d.hpp>
#include <visualization_msgs/msg/marker_array.hpp>
#include <std_srvs/srv/trigger.hpp>

#include <robot_msgs/msg/game_state.hpp>

#include "environment/GameEnvironment.hpp"
#include "game/GameLogic.hpp"

class GameNode : public rclcpp::Node {
public:
    GameNode();

private:
    void p1PoseCallback(const geometry_msgs::msg::Pose2D::SharedPtr msg);
    void p2PoseCallback(const geometry_msgs::msg::Pose2D::SharedPtr msg);
    void gameTickTimer();

    void publishWasteMarkers();
    void publishGameState();
    void publishScoreboard();

    void resetService(
        const std::shared_ptr<std_srvs::srv::Trigger::Request> req,
        std::shared_ptr<std_srvs::srv::Trigger::Response> res);

    // Logika
    std::shared_ptr<environment::GameEnvironment> game_env_;
    std::unique_ptr<game::GameLogic>              logic_;

    // ROS
    rclcpp::Subscription<geometry_msgs::msg::Pose2D>::SharedPtr p1_pose_sub_;
    rclcpp::Subscription<geometry_msgs::msg::Pose2D>::SharedPtr p2_pose_sub_;
    rclcpp::Publisher<visualization_msgs::msg::MarkerArray>::SharedPtr waste_pub_;
    rclcpp::Publisher<visualization_msgs::msg::MarkerArray>::SharedPtr scoreboard_pub_;
    rclcpp::Publisher<robot_msgs::msg::GameState>::SharedPtr state_pub_;
    rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr reset_srv_;
    rclcpp::TimerBase::SharedPtr tick_timer_;

    std::mutex robot_mutex_;
    double p1_x_ = 0.0, p1_y_ = 0.0;
    double p2_x_ = 0.0, p2_y_ = 0.0;
    double robot_radius_ = 0.3;

    std::string world_frame_;
    int spawn_count_initial_ = 15;
    double tick_dt_ = 0.1;
    bool announced_winner_ = false;
    int scoreboard_tick_ = 0;
    static constexpr int scoreboard_interval_ = 5;  // každý 5. tick pri 10 Hz = 2 Hz
};