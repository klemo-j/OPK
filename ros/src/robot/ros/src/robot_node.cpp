// ─────────────────────────────────────────────────────────────────────────────
// robot_node.cpp – ROS2 wrapper okolo robot::Robot
// Štart pozícia sa berie z parametrov (start_x, start_y, start_theta) aby
// to fungovalo pre P1 aj P2.
// ─────────────────────────────────────────────────────────────────────────────

#include "robot_node/RobotNode.hpp"

#include <chrono>
#include <memory>

#include <geometry_msgs/msg/transform_stamped.hpp>
#include <tf2/LinearMath/Quaternion.h>

RobotNode::RobotNode() : Node("robot_node")
{
    // ── ROS parametre ────────────────────────────────────────────────────
    declare_parameter<std::string>("config_file",
        "src/robot/cpp/config/environment.yaml");
    declare_parameter<int>("simulation_period_ms", 50);
    declare_parameter<double>("max_linear_acc",  1.0);
    declare_parameter<double>("max_angular_acc", 2.0);
    declare_parameter<double>("command_duration", 0.5);
    declare_parameter<std::string>("world_frame", "map");
    declare_parameter<std::string>("robot_frame", "base_link");
    declare_parameter<int>("publish_rate_hz", 20);

    // Štart – cez parameter (pre P1/P2). Ak nie je nastavené, použije sa z YAMLu.
    declare_parameter<double>("start_x", 999.0);   // 999 = "neset"
    declare_parameter<double>("start_y", 999.0);
    declare_parameter<double>("start_theta", 0.0);

    declare_parameter<std::string>("game_state_topic", "/game/state");

    const std::string config_file = get_parameter("config_file").as_string();
    const int sim_period          = get_parameter("simulation_period_ms").as_int();
    const double max_lin_acc      = get_parameter("max_linear_acc").as_double();
    const double max_ang_acc      = get_parameter("max_angular_acc").as_double();
    const double cmd_duration     = get_parameter("command_duration").as_double();
    world_frame_                  = get_parameter("world_frame").as_string();
    robot_frame_                  = get_parameter("robot_frame").as_string();
    const int rate_hz             = get_parameter("publish_rate_hz").as_int();

    const double param_start_x    = get_parameter("start_x").as_double();
    const double param_start_y    = get_parameter("start_y").as_double();
    const double param_start_th   = get_parameter("start_theta").as_double();

    const std::string game_state_topic = get_parameter("game_state_topic").as_string();

    // ── Postaviť čistú logiku ────────────────────────────────────────────
    try {
        auto cfg = environment::loadConfigFromYaml(config_file);
        game_env_ = std::make_shared<environment::GameEnvironment>(cfg);
    } catch (const std::exception& e) {
        RCLCPP_FATAL(get_logger(), "Chyba pri parsovani YAML: %s", e.what());
        throw;
    }

    const double radius = game_env_->getRobotRadius();

    robot::Robot::CollisionCb collision_cb =
        [this, radius](geometry::RobotState s) -> bool {
            return game_env_->isCollision(s.x, s.y, radius);
        };

    // Štart pozícia – uprednostni parameter, fallback na YAML p1
    double sx, sy, sth;
    if (param_start_x < 998.0) {
        sx = param_start_x;
        sy = param_start_y;
        sth = param_start_th;
    } else {
        const auto& p = game_env_->getP1Start();
        sx = p.x; sy = p.y; sth = p.theta;
    }

    robot::Config robot_cfg;
    robot_cfg.accelerations           = { max_lin_acc, max_ang_acc };
    robot_cfg.emergency_decelerations = { max_lin_acc * 2.0, max_ang_acc * 2.0 };
    robot_cfg.command_duration        = cmd_duration;
    robot_cfg.simulation_period_ms    = sim_period;
    robot_cfg.start_x                 = sx;
    robot_cfg.start_y                 = sy;
    robot_cfg.start_theta             = sth;

    robot_ = std::make_unique<robot::Robot>(robot_cfg, collision_cb);

    // ── ROS publishery ───────────────────────────────────────────────────
    pose_pub_         = create_publisher<geometry_msgs::msg::Pose2D>("robot/pose", 10);
    pose_stamped_pub_ = create_publisher<geometry_msgs::msg::PoseStamped>("robot/pose_stamped", 10);
    twist_pub_        = create_publisher<geometry_msgs::msg::Twist>("robot/twist", 10);
    path_pub_         = create_publisher<nav_msgs::msg::Path>("robot/path", 10);

    path_.header.frame_id = world_frame_;

    // ── Subscriber na cmd_vel ────────────────────────────────────────────
    cmd_sub_ = create_subscription<geometry_msgs::msg::Twist>(
        "robot/cmd_vel", 10,
        [this](geometry_msgs::msg::Twist::SharedPtr msg) {
            cmdVelCallback(msg);
        });

    // ── Subscriber na game state (kvôli game_over stop) ──────────────────
    game_state_sub_ = create_subscription<robot_msgs::msg::GameState>(
        game_state_topic, 10,
        [this](robot_msgs::msg::GameState::SharedPtr msg) {
            gameStateCallback(msg);
        });

    tf_broadcaster_ = std::make_unique<tf2_ros::TransformBroadcaster>(*this);

    auto period = std::chrono::milliseconds(1000 / rate_hz);
    state_timer_ = create_wall_timer(period, [this]() {
        publishStateTimer();
    });

    RCLCPP_INFO(get_logger(),
        "RobotNode pripraveny. Stat: (%.2f, %.2f, %.2f)", sx, sy, sth);
}

void RobotNode::gameStateCallback(const robot_msgs::msg::GameState::SharedPtr msg) {
    if (msg->game_over && !game_over_.load()) {
        game_over_.store(true);
        // Zastavíme robota
        robot_->setVelocity({0.0, 0.0});
        RCLCPP_INFO(get_logger(), "Hra skoncila, robot zastaveny.");
    }
}

void RobotNode::cmdVelCallback(const geometry_msgs::msg::Twist::SharedPtr msg)
{
    RCLCPP_INFO(get_logger(), "CMD VEL: linear=%.2f, angular=%.2f, game_over=%d",
                msg->linear.x, msg->angular.z, game_over_.load() ? 1 : 0);

    geometry::Twist t;
    t.linear  = msg->linear.x;
    t.angular = msg->angular.z;
    robot_->setVelocity(t);
}

void RobotNode::publishStateTimer()
{
    geometry::RobotState s = robot_->getState();

    tf2::Quaternion q;
    q.setRPY(0, 0, s.theta);

    // pose
    geometry_msgs::msg::Pose2D pose_msg;
    pose_msg.x     = s.x;
    pose_msg.y     = s.y;
    pose_msg.theta = s.theta;
    pose_pub_->publish(pose_msg);

    // PoseStamped
    geometry_msgs::msg::PoseStamped ps;
    ps.header.stamp    = now();
    ps.header.frame_id = world_frame_;
    ps.pose.position.x = s.x;
    ps.pose.position.y = s.y;
    ps.pose.position.z = 0.0;
    ps.pose.orientation.x = q.x();
    ps.pose.orientation.y = q.y();
    ps.pose.orientation.z = q.z();
    ps.pose.orientation.w = q.w();
    pose_stamped_pub_->publish(ps);

    // Path
    path_.header.stamp = now();
    path_.poses.push_back(ps);
    if (path_.poses.size() > 5000) {
        path_.poses.erase(path_.poses.begin(),
                          path_.poses.begin() + 1000);
    }
    path_pub_->publish(path_);

    // Twist
    geometry_msgs::msg::Twist twist_msg;
    twist_msg.linear.x  = s.velocity.linear;
    twist_msg.angular.z = s.velocity.angular;
    twist_pub_->publish(twist_msg);

    // TF
    geometry_msgs::msg::TransformStamped tf;
    tf.header.stamp    = now();
    tf.header.frame_id = world_frame_;
    tf.child_frame_id  = robot_frame_;
    tf.transform.translation.x = s.x;
    tf.transform.translation.y = s.y;
    tf.transform.translation.z = 0.0;
    tf.transform.rotation.x = q.x();
    tf.transform.rotation.y = q.y();
    tf.transform.rotation.z = q.z();
    tf.transform.rotation.w = q.w();
    tf_broadcaster_->sendTransform(tf);
}

int main(int argc, char** argv)
{
    rclcpp::init(argc, argv);
    try {
        auto node = std::make_shared<RobotNode>();
        rclcpp::spin(node);
    } catch (const std::exception& e) {
        std::fprintf(stderr, "robot_node padol: %s\n", e.what());
    }
    rclcpp::shutdown();
    return 0;
}