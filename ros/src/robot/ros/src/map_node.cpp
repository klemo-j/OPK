// ─────────────────────────────────────────────────────────────────────────────
// map_node.cpp
// ─────────────────────────────────────────────────────────────────────────────
// Publikuje statickú mapu a markery prekážok / stanice pre RViz.
// Vizualizácia samotná (kreslenie) je v RViz, tu len pripravujeme dáta.
// ─────────────────────────────────────────────────────────────────────────────

#include "map_node/MapNode.hpp"

#include <chrono>

#include <visualization_msgs/msg/marker.hpp>

MapNode::MapNode() : Node("map_node") {
    declare_parameter<std::string>("config_file",
        "src/robot/cpp/config/environment.yaml");
    declare_parameter<std::string>("world_frame", "map");
    declare_parameter<int>("publish_rate_hz", 1);

    const std::string cfg_file = get_parameter("config_file").as_string();
    world_frame_                = get_parameter("world_frame").as_string();
    const int rate_hz           = get_parameter("publish_rate_hz").as_int();

    try {
        auto cfg = environment::loadConfigFromYaml(cfg_file);
        game_env_ = std::make_shared<environment::GameEnvironment>(cfg);
    } catch (const std::exception& e) {
        RCLCPP_FATAL(get_logger(), "YAML chyba: %s", e.what());
        throw;
    }

    map_pub_    = create_publisher<nav_msgs::msg::OccupancyGrid>("map", 1);
    marker_pub_ = create_publisher<visualization_msgs::msg::MarkerArray>(
        "environment/markers", 1);

    cached_grid_    = buildOccupancyGrid();
    cached_markers_ = buildMarkerArray();

    auto period = std::chrono::milliseconds(1000 / rate_hz);
    timer_ = create_wall_timer(period, [this]() { publishAll(); });

    RCLCPP_INFO(get_logger(),
        "MapNode pripravený. Mapa %dx%d, rozlíšenie %.3f m/px, "
        "%zu prekážok.",
        cached_grid_.info.width, cached_grid_.info.height,
        cached_grid_.info.resolution,
        game_env_->getObstacles().size());
}

void MapNode::publishAll() {
    cached_grid_.header.stamp = now();
    map_pub_->publish(cached_grid_);

    auto stamp = now();
    for (auto& m : cached_markers_.markers) m.header.stamp = stamp;
    marker_pub_->publish(cached_markers_);
}

// ─────────────────────────────────────────────────────────────────────────────
// OccupancyGrid – konvertuje PNG mapu na ROS formát
// ─────────────────────────────────────────────────────────────────────────────
nav_msgs::msg::OccupancyGrid MapNode::buildOccupancyGrid() const {
    nav_msgs::msg::OccupancyGrid grid;
    grid.header.frame_id = world_frame_;

    grid.info.resolution = game_env_->getResolution();
    grid.info.width      = game_env_->getMapWidthPx();
    grid.info.height     = game_env_->getMapHeightPx();

    // Origin = ľavý dolný roh mapy v svetových súradniciach.
    // Naša konvencia: svet (0,0) = ľavý dolný roh mapy. Takže origin = 0,0.
    // Stred mapy = (0, 0). Origin = ľavý dolný roh = (-w/2, -h/2)
    grid.info.origin.position.x = -(grid.info.width  * grid.info.resolution) / 2.0;
    grid.info.origin.position.y = -(grid.info.height * grid.info.resolution) / 2.0;
    grid.info.origin.orientation.w = 1.0;

    const cv::Mat& img = game_env_->getMapImage();
    grid.data.resize(grid.info.width * grid.info.height, 0);

    // Konverzia: tmavý pixel (<128) = obsadený (100), inak voľné (0).
    // PNG má y zhora nadol, ROS chce y zdola nahor → flipneme riadky.
    for (unsigned y = 0; y < grid.info.height; ++y) {
        const uint8_t* row = img.ptr<uint8_t>(grid.info.height - 1 - y);
        for (unsigned x = 0; x < grid.info.width; ++x) {
            grid.data[y * grid.info.width + x] = (row[x] < 128) ? 100 : 0;
        }
    }

    return grid;
}

// ─────────────────────────────────────────────────────────────────────────────
// MarkerArray – prekážky (červené) a stanica (zelená)
// ─────────────────────────────────────────────────────────────────────────────
visualization_msgs::msg::MarkerArray MapNode::buildMarkerArray() const {
    visualization_msgs::msg::MarkerArray arr;
    int id = 0;

    // ── Prekážky ──────────────────────────────────────────────────────────
    for (const auto& obs : game_env_->getObstacles()) {
        visualization_msgs::msg::Marker m;
        m.header.frame_id = world_frame_;
        m.ns              = "obstacles";
        m.id              = id++;
        m.action          = visualization_msgs::msg::Marker::ADD;
        m.color.r = 1.0;  m.color.g = 0.0;  m.color.b = 0.0;  m.color.a = 1.0;

        if (auto* circle = dynamic_cast<const game::CircleObstacle*>(obs.get())) {
            m.type = visualization_msgs::msg::Marker::CYLINDER;
            m.pose.position.x = circle->getPosition().x;
            m.pose.position.y = circle->getPosition().y;
            m.pose.position.z = 0.5;
            m.pose.orientation.w = 1.0;
            const double d = circle->getRadius() * 2.0;
            m.scale.x = d;  m.scale.y = d;  m.scale.z = 1.0;

        } else if (auto* rect =
                   dynamic_cast<const game::RectangleObstacle*>(obs.get())) {
            m.type = visualization_msgs::msg::Marker::CUBE;
            // Pozícia rectangle v Marker je STRED, ale my máme ľavý-dolný roh.
            const double w = rect->getWidth();
            const double h = rect->getHeight();
            m.pose.position.x = rect->getPosition().x + w / 2.0;
            m.pose.position.y = rect->getPosition().y + h / 2.0;
            m.pose.position.z = 0.5;
            m.pose.orientation.w = 1.0;
            m.scale.x = w;  m.scale.y = h;  m.scale.z = 1.0;
        } else {
            continue;   // neznámy typ – preskočiť
        }
        arr.markers.push_back(m);
    }

    // ── Stanica (zelený valec) ────────────────────────────────────────────
    {
        const auto& st = game_env_->getStation();
        visualization_msgs::msg::Marker m;
        m.header.frame_id = world_frame_;
        m.ns              = "station";
        m.id              = id++;
        m.type            = visualization_msgs::msg::Marker::CYLINDER;
        m.action          = visualization_msgs::msg::Marker::ADD;
        m.pose.position.x = st.getPosition().x;
        m.pose.position.y = st.getPosition().y;
        m.pose.position.z = 0.05;
        m.pose.orientation.w = 1.0;
        const double d = st.getRadius() * 2.0;
        m.scale.x = d;  m.scale.y = d;  m.scale.z = 0.1;
        m.color.r = 0.0; m.color.g = 1.0; m.color.b = 0.0; m.color.a = 0.7;
        arr.markers.push_back(m);
    }

    return arr;
}

int main(int argc, char** argv) {
    rclcpp::init(argc, argv);
    try {
        auto node = std::make_shared<MapNode>();
        rclcpp::spin(node);
    } catch (const std::exception& e) {
        std::fprintf(stderr, "map_node padol: %s\n", e.what());
    }
    rclcpp::shutdown();
    return 0;
}