// ─────────────────────────────────────────────────────────────────────────────
// game_node.cpp – Duel mód: 2 hráči, 60s, spoločný pool odpadkov a stanica
// ─────────────────────────────────────────────────────────────────────────────

#include "game_node/GameNode.hpp"

#include <chrono>
#include <sstream>
#include <iomanip>
#include <visualization_msgs/msg/marker.hpp>

GameNode::GameNode() : Node("game_node")
{
    // ── Parametre ────────────────────────────────────────────────────────
    declare_parameter<std::string>("config_file",
        "src/robot/cpp/config/environment.yaml");
    declare_parameter<std::string>("world_frame", "map");
    declare_parameter<int>("game_tick_hz", 10);
    declare_parameter<int>("random_seed", 42);
    declare_parameter<std::string>("p1_pose_topic", "/p1/robot/pose");
    declare_parameter<std::string>("p2_pose_topic", "/p2/robot/pose");

    const std::string config_file    = get_parameter("config_file").as_string();
    world_frame_                     = get_parameter("world_frame").as_string();
    const int tick_hz                = get_parameter("game_tick_hz").as_int();
    const int seed                   = get_parameter("random_seed").as_int();
    const std::string p1_topic       = get_parameter("p1_pose_topic").as_string();
    const std::string p2_topic       = get_parameter("p2_pose_topic").as_string();

    tick_dt_ = 1.0 / static_cast<double>(tick_hz);

    // ── Postaviť herné prostredie a logiku ───────────────────────────────
    try {
        auto cfg = environment::loadConfigFromYaml(config_file);
        game_env_ = std::make_shared<environment::GameEnvironment>(cfg);

        robot_radius_         = game_env_->getRobotRadius();
        spawn_count_initial_  = game_env_->getGameSettings().initial_count;
        const double duration = game_env_->getGameSettings().duration_sec;

        waste::WasteGeneratorConfig gen_cfg;
        gen_cfg.radius_min = game_env_->getWasteConfig().radius_min;
        gen_cfg.radius_max = game_env_->getWasteConfig().radius_max;
        gen_cfg.map_width  = game_env_->getWidth();
        gen_cfg.map_height = game_env_->getHeight();

        logic_ = std::make_unique<game::GameLogic>(
            game_env_, gen_cfg, game_env_->getMaxCapacity(),
            duration, seed);

        RCLCPP_INFO(get_logger(),
            "Duel: trvanie %.0fs, %d odpadkov pri starte, kapacita %d.",
            duration, spawn_count_initial_, game_env_->getMaxCapacity());
    } catch (const std::exception& e) {
        RCLCPP_FATAL(get_logger(), "YAML chyba: %s", e.what());
        throw;
    }

    logic_->spawnWastes(spawn_count_initial_);

    // ── ROS rozhranie ────────────────────────────────────────────────────
    p1_pose_sub_ = create_subscription<geometry_msgs::msg::Pose2D>(
        p1_topic, 10,
        [this](geometry_msgs::msg::Pose2D::SharedPtr msg) {
            p1PoseCallback(msg);
        });

    p2_pose_sub_ = create_subscription<geometry_msgs::msg::Pose2D>(
        p2_topic, 10,
        [this](geometry_msgs::msg::Pose2D::SharedPtr msg) {
            p2PoseCallback(msg);
        });

    waste_pub_ = create_publisher<visualization_msgs::msg::MarkerArray>(
        "game/waste_markers", 1);

    scoreboard_pub_ = create_publisher<visualization_msgs::msg::MarkerArray>(
        "game/scoreboard", 1);

    state_pub_ = create_publisher<robot_msgs::msg::GameState>(
        "game/state", 10);

    reset_srv_ = create_service<std_srvs::srv::Trigger>(
        "game/reset",
        std::bind(&GameNode::resetService, this,
                  std::placeholders::_1, std::placeholders::_2));

    auto period = std::chrono::milliseconds(1000 / tick_hz);
    tick_timer_ = create_wall_timer(period, [this]() { gameTickTimer(); });

    RCLCPP_INFO(get_logger(),
        "GameNode pripraveny. Topicy: %s, %s",
        p1_topic.c_str(), p2_topic.c_str());
}

void GameNode::p1PoseCallback(const geometry_msgs::msg::Pose2D::SharedPtr msg) {
    std::lock_guard<std::mutex> lock(robot_mutex_);
    p1_x_ = msg->x;
    p1_y_ = msg->y;
}

void GameNode::p2PoseCallback(const geometry_msgs::msg::Pose2D::SharedPtr msg) {
    std::lock_guard<std::mutex> lock(robot_mutex_);
    p2_x_ = msg->x;
    p2_y_ = msg->y;
}

void GameNode::gameTickTimer() {
    double p1x, p1y, p2x, p2y, rr;
    {
        std::lock_guard<std::mutex> lock(robot_mutex_);
        p1x = p1_x_; p1y = p1_y_;
        p2x = p2_x_; p2y = p2_y_;
        rr  = robot_radius_;
    }

    logic_->update(p1x, p1y, p2x, p2y, rr, tick_dt_);

    publishWasteMarkers();
    publishGameState();

    // Scoreboard stačí 2× za sekundu – znižuje vizuálne blikanie textu
    scoreboard_tick_++;
    if (scoreboard_tick_ >= scoreboard_interval_) {
        scoreboard_tick_ = 0;
        publishScoreboard();
    }

    // Oznámiť víťaza jedenkrát do konzoly
    const auto& s = logic_->getState();
    if (s.game_over && !announced_winner_) {
        announced_winner_ = true;
        RCLCPP_INFO(get_logger(), "========================================");
        RCLCPP_INFO(get_logger(), "    HRA SKONCILA!");
        RCLCPP_INFO(get_logger(), "    P1: %d  |  P2: %d", s.p1.score, s.p2.score);
        if (s.winner == "p1")
            RCLCPP_INFO(get_logger(), "    VITAZ: HRAC 1 (sipky)");
        else if (s.winner == "p2")
            RCLCPP_INFO(get_logger(), "    VITAZ: HRAC 2 (WASD)");
        else
            RCLCPP_INFO(get_logger(), "    REMIZA");
        RCLCPP_INFO(get_logger(), "========================================");
    }
}

void GameNode::publishWasteMarkers() {
    visualization_msgs::msg::MarkerArray arr;
    int id = 1;

    // DELETEALL aby sa staré markery zmazali (id=0)
    {
        visualization_msgs::msg::Marker del;
        del.header.frame_id = world_frame_;
        del.header.stamp = now();
        del.ns = "wastes";
        del.id = 0;
        del.action = visualization_msgs::msg::Marker::DELETEALL;
        arr.markers.push_back(del);
    }

    for (const auto& w : logic_->getActiveWastes()) {
        visualization_msgs::msg::Marker m;
        m.header.frame_id = world_frame_;
        m.header.stamp = now();
        m.ns   = "wastes";
        m.id   = id++;
        m.type = visualization_msgs::msg::Marker::CYLINDER;
        m.action = visualization_msgs::msg::Marker::ADD;
        m.pose.position.x = w->getPosition().x;
        m.pose.position.y = w->getPosition().y;
        m.pose.position.z = 0.05;
        m.pose.orientation.w = 1.0;
        const double d = w->getRadius() * 2.0;
        m.scale.x = d;  m.scale.y = d;  m.scale.z = 0.1;

        switch (w->getType()) {
            case waste::WasteType::PAPER:
                m.color.r = 1.0; m.color.g = 1.0; m.color.b = 0.4; break;
            case waste::WasteType::PLASTIC:
                m.color.r = 1.0; m.color.g = 0.8; m.color.b = 0.0; break;
            case waste::WasteType::GLASS:
                m.color.r = 0.9; m.color.g = 0.9; m.color.b = 0.2; break;
        }
        m.color.a = 1.0;

        arr.markers.push_back(m);
    }

    waste_pub_->publish(arr);
}

void GameNode::publishScoreboard() {
    const auto& s = logic_->getState();
    visualization_msgs::msg::MarkerArray arr;

    // Pomocná lambda – vytvorí TEXT_VIEW_FACING marker na danej pozícii
    auto make_text = [&](int id, const std::string& text,
                         double x, double y, double z,
                         float r, float g, float b, double size = 0.6)
    {
        visualization_msgs::msg::Marker m;
        m.header.frame_id = world_frame_;
        m.header.stamp    = now();
        m.ns     = "scoreboard";
        m.id     = id;
        m.type   = visualization_msgs::msg::Marker::TEXT_VIEW_FACING;
        m.action = visualization_msgs::msg::Marker::ADD;
        m.pose.position.x = x;
        m.pose.position.y = y;
        m.pose.position.z = z;
        m.pose.orientation.w = 1.0;
        m.scale.z  = size;
        m.color.r  = r;  m.color.g = g;  m.color.b = b;  m.color.a = 1.0;
        m.text     = text;
        // Lifetime 1.5s – marker zmizne ak ho node prestane posielat
        m.lifetime = rclcpp::Duration(1, 500'000'000);
        return m;
    };

    // Scoreboard je umiestený NAD mapou (mapa ide do y≈10.24, my dávame y=12.5)
    // a vycentrovaný horizontálne.
    const double X = 0.0, Y_TOP = 12.5, DY = 1.1;
    arr.markers.push_back(make_text(0, "=== SCOREBOARD ===",
        X, Y_TOP, 0.5, 1.0f, 0.85f, 0.0f, 0.7));

    // Čas
    std::ostringstream time_ss;
    if (s.game_over) {
        time_ss << "CAS: KONIEC";
    } else {
        time_ss << "CAS: " << std::fixed << std::setprecision(1) << s.time_left << " s";
    }
    arr.markers.push_back(make_text(1, time_ss.str(),
        X, Y_TOP - DY, 0.5, 1.0f, 1.0f, 1.0f));

    // P1
    std::ostringstream p1_ss;
    p1_ss << "P1 (sipky): " << s.p1.score << " bodov"
          << "  |  batoh: " << s.p1.capacity_used << "/" << s.p1.capacity_max;
    arr.markers.push_back(make_text(2, p1_ss.str(),
        X, Y_TOP - 2*DY, 0.5, 0.2f, 0.6f, 1.0f));

    // P2
    std::ostringstream p2_ss;
    p2_ss << "P2 (WASD):  " << s.p2.score << " bodov"
          << "  |  batoh: " << s.p2.capacity_used << "/" << s.p2.capacity_max;
    arr.markers.push_back(make_text(3, p2_ss.str(),
        X, Y_TOP - 3*DY, 0.5, 0.85f, 0.2f, 1.0f));

    // Víťaz / stav hry
    if (s.game_over) {
        std::string winner_text;
        if (s.winner == "p1")       winner_text = ">> VITAZ: P1 (sipky) <<";
        else if (s.winner == "p2")  winner_text = ">> VITAZ: P2 (WASD)  <<";
        else                        winner_text = ">>      REMIZA       <<";
        arr.markers.push_back(make_text(4, winner_text,
            X, Y_TOP - 4*DY, 0.5, 1.0f, 0.3f, 0.3f, 0.8));
    } else {
        // Odpadky zostatok
        std::ostringstream waste_ss;
        waste_ss << "Odpadky: " << s.waste_in_world << " v mape";
        arr.markers.push_back(make_text(4, waste_ss.str(),
            X, Y_TOP - 4*DY, 0.5, 0.6f, 0.9f, 0.4f));
    }

    scoreboard_pub_->publish(arr);
}

void GameNode::publishGameState() {
    const auto& s = logic_->getState();

    auto cnt = [](const std::map<waste::WasteType, int>& m, waste::WasteType t) {
        auto it = m.find(t);
        return it != m.end() ? it->second : 0;
    };

    robot_msgs::msg::GameState msg;
    msg.time_left          = s.time_left;
    msg.game_over          = s.game_over;
    msg.winner             = s.winner;

    msg.p1_score           = s.p1.score;
    msg.p1_capacity_used   = s.p1.capacity_used;
    msg.p1_capacity_max    = s.p1.capacity_max;
    msg.p1_paper           = cnt(s.p1.collected_counts, waste::WasteType::PAPER);
    msg.p1_plastic         = cnt(s.p1.collected_counts, waste::WasteType::PLASTIC);
    msg.p1_glass           = cnt(s.p1.collected_counts, waste::WasteType::GLASS);

    msg.p2_score           = s.p2.score;
    msg.p2_capacity_used   = s.p2.capacity_used;
    msg.p2_capacity_max    = s.p2.capacity_max;
    msg.p2_paper           = cnt(s.p2.collected_counts, waste::WasteType::PAPER);
    msg.p2_plastic         = cnt(s.p2.collected_counts, waste::WasteType::PLASTIC);
    msg.p2_glass           = cnt(s.p2.collected_counts, waste::WasteType::GLASS);

    msg.wastes_remaining   = s.waste_in_world;

    state_pub_->publish(msg);
}

void GameNode::resetService(
    const std::shared_ptr<std_srvs::srv::Trigger::Request> /*req*/,
    std::shared_ptr<std_srvs::srv::Trigger::Response> res)
{
    logic_->reset();
    logic_->spawnWastes(spawn_count_initial_);
    announced_winner_ = false;
    res->success = true;
    res->message = "Hra resetnuta.";
    RCLCPP_INFO(get_logger(), "Hra resetnuta.");
}

int main(int argc, char** argv) {
    rclcpp::init(argc, argv);
    try {
        auto node = std::make_shared<GameNode>();
        rclcpp::spin(node);
    } catch (const std::exception& e) {
        std::fprintf(stderr, "game_node padol: %s\n", e.what());
        std::fprintf(stderr, "game_node padol: %s\n", e.what());

    }
    rclcpp::shutdown();
    return 0;
}