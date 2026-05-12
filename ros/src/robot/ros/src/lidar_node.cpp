// ─────────────────────────────────────────────────────────────────────────────
// lidar_node.cpp – ROS2 wrapper okolo lidar::Lidar
// ─────────────────────────────────────────────────────────────────────────────

#include "lidar_node/LidarNode.hpp"

#include <chrono>
#include <cmath>
#include <memory>
#include <mutex>

#include <geometry_msgs/msg/transform_stamped.hpp>
#include <tf2/LinearMath/Quaternion.h>

LidarNode::LidarNode() : Node("lidar_node") {
    // ── 1. Parametre ──────────────────────────────────────────────────────
    declare_parameter<std::string>("config_file",
        "src/robot/cpp/config/environment.yaml");
    declare_parameter<double>("max_range", 5.0);
    declare_parameter<int>("beam_count", 360);
    declare_parameter<double>("first_ray_angle", -M_PI);
    declare_parameter<double>("last_ray_angle",   M_PI);
    declare_parameter<int>("scan_rate_hz", 10);
    declare_parameter<std::string>("laser_frame", "laser");
    declare_parameter<std::string>("robot_frame", "base_link");

    const std::string config_file = get_parameter("config_file").as_string();
    lidar_cfg_.max_range       = get_parameter("max_range").as_double();
    lidar_cfg_.beam_count      = get_parameter("beam_count").as_int();
    lidar_cfg_.first_ray_angle = get_parameter("first_ray_angle").as_double();
    lidar_cfg_.last_ray_angle  = get_parameter("last_ray_angle").as_double();
    const int rate_hz          = get_parameter("scan_rate_hz").as_int();
    laser_frame_               = get_parameter("laser_frame").as_string();
    robot_frame_               = get_parameter("robot_frame").as_string();

    // ── 2. Postaviť environment a lidar ───────────────────────────────────
    try {
        auto cfg = environment::loadConfigFromYaml(config_file);
        game_env_ = std::make_shared<environment::GameEnvironment>(cfg);
    } catch (const std::exception& e) {
        RCLCPP_FATAL(get_logger(), "YAML chyba: %s", e.what());
        throw;
    }

    lidar_ = std::make_unique<lidar::Lidar>(lidar_cfg_, game_env_);

    // ── 3. ROS rozhranie ──────────────────────────────────────────────────
    scan_pub_ = create_publisher<sensor_msgs::msg::LaserScan>("scan", 10);

    pose_sub_ = create_subscription<geometry_msgs::msg::Pose2D>(
        "robot/pose", 10,
        [this](geometry_msgs::msg::Pose2D::SharedPtr msg) {
            poseCallback(msg);
        });

    static_tf_ = std::make_unique<tf2_ros::StaticTransformBroadcaster>(*this);

    // Statická transformácia base_link → laser
    geometry_msgs::msg::TransformStamped tf;
    tf.header.stamp    = now();
    tf.header.frame_id = robot_frame_;
    tf.child_frame_id  = laser_frame_;
    tf.transform.translation.x = 0.0;
    tf.transform.translation.y = 0.0;
    tf.transform.translation.z = 0.0;
    tf.transform.rotation.w    = 1.0;
    static_tf_->sendTransform(tf);

    auto period = std::chrono::milliseconds(1000 / rate_hz);
    scan_timer_ = create_wall_timer(period, [this]() { scanTimer(); });

    RCLCPP_INFO(get_logger(),
        "LidarNode pripravený: %d lúčov, %.1f m, frame='%s'",
        lidar_cfg_.beam_count, lidar_cfg_.max_range, laser_frame_.c_str());
}

void LidarNode::poseCallback(const geometry_msgs::msg::Pose2D::SharedPtr msg) {
    std::lock_guard<std::mutex> lock(state_mutex_);
    last_state_.x     = msg->x;
    last_state_.y     = msg->y;
    last_state_.theta = msg->theta;
}

void LidarNode::scanTimer() {
    geometry::RobotState s;
    {
        std::lock_guard<std::mutex> lock(state_mutex_);
        s = last_state_;
    }

    auto ranges = lidar_->scan(s);

    sensor_msgs::msg::LaserScan scan_msg;
    scan_msg.header.stamp    = now();
    scan_msg.header.frame_id = laser_frame_;
    scan_msg.angle_min       = lidar_cfg_.first_ray_angle;
    scan_msg.angle_max       = lidar_cfg_.last_ray_angle;
    scan_msg.angle_increment =
        (lidar_cfg_.beam_count > 1)
        ? (lidar_cfg_.last_ray_angle - lidar_cfg_.first_ray_angle) /
          (lidar_cfg_.beam_count - 1)
        : 0.0;
    scan_msg.range_min = 0.0;
    scan_msg.range_max = static_cast<float>(lidar_cfg_.max_range);
    scan_msg.ranges.resize(lidar_cfg_.beam_count, scan_msg.range_max);

    for (int i = 0; i < lidar_cfg_.beam_count; ++i) {
        scan_msg.ranges[i] = static_cast<float>(ranges[i]);
    }

    scan_pub_->publish(scan_msg);
}

int main(int argc, char** argv) {
    rclcpp::init(argc, argv);
    try {
        auto node = std::make_shared<LidarNode>();
        rclcpp::spin(node);
    } catch (const std::exception& e) {
        std::fprintf(stderr, "lidar_node padol: %s\n", e.what());
    }
    rclcpp::shutdown();
    return 0;
}