#pragma once

// ─────────────────────────────────────────────────────────────────────────────
// MapNode
// ─────────────────────────────────────────────────────────────────────────────
// Publikuje:
//   /map                    nav_msgs/OccupancyGrid     (PNG mapa)
//   /environment/markers    visualization_msgs/MarkerArray (prekážky + stanica)
//
// Dáta sú statické – prečítajú sa raz z YAMLu, potom sa periodicky publikujú,
// aby aj nový subscriber (napr. RViz spustený neskôr) dostal aktuálny obraz.
// ─────────────────────────────────────────────────────────────────────────────

#include <memory>

#include <rclcpp/rclcpp.hpp>
#include <nav_msgs/msg/occupancy_grid.hpp>
#include <visualization_msgs/msg/marker_array.hpp>

#include "environment/GameEnvironment.hpp"

class MapNode : public rclcpp::Node {
public:
    MapNode();

private:
    void publishAll();

    // Postaví OccupancyGrid raz na začiatku
    nav_msgs::msg::OccupancyGrid buildOccupancyGrid() const;
    visualization_msgs::msg::MarkerArray buildMarkerArray() const;

    std::shared_ptr<environment::GameEnvironment> game_env_;

    rclcpp::Publisher<nav_msgs::msg::OccupancyGrid>::SharedPtr        map_pub_;
    rclcpp::Publisher<visualization_msgs::msg::MarkerArray>::SharedPtr marker_pub_;
    rclcpp::TimerBase::SharedPtr                                      timer_;

    nav_msgs::msg::OccupancyGrid          cached_grid_;
    visualization_msgs::msg::MarkerArray  cached_markers_;

    std::string world_frame_;
};