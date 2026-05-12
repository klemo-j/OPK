#pragma once

#include "environment/Environment.hpp"
#include "types/GameObjects.hpp"
#include <vector>
#include <memory>
#include <string>

namespace environment {

// ─────────────────────────────────────────────────────────────────────────────
// Konfiguračné štruktúry (z YAML)
// ─────────────────────────────────────────────────────────────────────────────
struct ObstacleConfig {
    std::string type;       // "circle" alebo "rectangle"
    double x = 0.0;
    double y = 0.0;
    double radius = 0.0;
    double width  = 0.0;
    double height = 0.0;
};

struct StationConfig {
    double x      = 0.0;
    double y      = 0.0;
    double radius = 0.5;
};

struct PlayerStart {
    double x = 0.0;
    double y = 0.0;
    double theta = 0.0;
};

struct RobotGameConfig {
    double radius       = 0.30;
    int    max_capacity = 5;
};

struct WasteConfig {
    double radius_min = 0.10;
    double radius_max = 0.30;
    std::vector<std::string> types = {"paper", "plastic", "glass"};
};

struct GameSettings {
    double duration_sec = 60.0;
    int    initial_count = 15;
};

struct GameEnvironmentConfig {
    std::string map_filename;
    double      resolution = 0.05;

    std::vector<ObstacleConfig> obstacles;
    StationConfig               station;
    RobotGameConfig             robot;
    WasteConfig                 waste;
    GameSettings                game;
    PlayerStart                 p1;
    PlayerStart                 p2;
};

// Parser YAML → GameEnvironmentConfig
GameEnvironmentConfig loadConfigFromYaml(const std::string& yaml_path);

// ─────────────────────────────────────────────────────────────────────────────
// GameEnvironment – Environment + prekážky + stanica
// ─────────────────────────────────────────────────────────────────────────────
class GameEnvironment : public Environment {
public:
    explicit GameEnvironment(const GameEnvironmentConfig& cfg);

    // Pre kolíziu robota: mapa + prekážky
    bool isCollision(double x, double y, double radius) const override;

    // Pre lidar: mapa (1 pixel) + prekážky
    bool isOccupied(double x, double y) const override;

    // Robot je v stanici?
    bool isAtStation(double x, double y, double radius) const;

    // Prístup pre map_node a iných
    const std::vector<std::unique_ptr<game::GameObject>>& getObstacles() const {
        return obstacles_;
    }
    const game::Station& getStation() const { return *station_; }

    double getRobotRadius() const { return cfg_.robot.radius; }
    int    getMaxCapacity() const { return cfg_.robot.max_capacity; }
    const RobotGameConfig& getRobotConfig() const { return cfg_.robot; }
    const WasteConfig&     getWasteConfig() const { return cfg_.waste; }
    const GameSettings&    getGameSettings() const { return cfg_.game; }
    const PlayerStart&     getP1Start() const { return cfg_.p1; }
    const PlayerStart&     getP2Start() const { return cfg_.p2; }

private:
    void bakeObstaclesIntoGrid();  // zakreslí geometrické prekážky do occupancy_

    GameEnvironmentConfig cfg_;
    std::vector<std::unique_ptr<game::GameObject>> obstacles_;
    std::unique_ptr<game::Station> station_;
};

} // namespace environment