#include "environment/GameEnvironment.hpp"
#include <yaml-cpp/yaml.h>
#include <stdexcept>

namespace environment {

// ─────────────────────────────────────────────────────────────────────────────
// Parser YAML → GameEnvironmentConfig
// ─────────────────────────────────────────────────────────────────────────────
GameEnvironmentConfig loadConfigFromYaml(const std::string& yaml_path) {
    YAML::Node root;
    try {
        root = YAML::LoadFile(yaml_path);
    } catch (const std::exception& e) {
        throw game::GameException(
            "YAML chyba pri otvoreni '" + yaml_path + "': " + e.what());
    }

    GameEnvironmentConfig cfg;

    // ── map ──
    if (!root["map"]) {
        throw game::GameException("YAML: chyba sekcia 'map'");
    }
    cfg.map_filename = root["map"]["filename"].as<std::string>();
    cfg.resolution   = root["map"]["resolution"].as<double>();

    // ── robot ──
    if (root["robot"]) {
        if (root["robot"]["radius"])
            cfg.robot.radius = root["robot"]["radius"].as<double>();
        if (root["robot"]["max_capacity"])
            cfg.robot.max_capacity = root["robot"]["max_capacity"].as<int>();
    }

    // ── players ──
    if (root["players"]) {
        if (root["players"]["p1"]) {
            cfg.p1.x     = root["players"]["p1"]["x"].as<double>();
            cfg.p1.y     = root["players"]["p1"]["y"].as<double>();
            cfg.p1.theta = root["players"]["p1"]["theta"].as<double>();
        }
        if (root["players"]["p2"]) {
            cfg.p2.x     = root["players"]["p2"]["x"].as<double>();
            cfg.p2.y     = root["players"]["p2"]["y"].as<double>();
            cfg.p2.theta = root["players"]["p2"]["theta"].as<double>();
        }
    }

    // ── station ──
    if (root["station"]) {
        cfg.station.x      = root["station"]["x"].as<double>();
        cfg.station.y      = root["station"]["y"].as<double>();
        cfg.station.radius = root["station"]["radius"].as<double>();
    }

    // ── obstacles ──
    if (root["obstacles"]) {
        for (const auto& o : root["obstacles"]) {
            ObstacleConfig oc;
            oc.type = o["type"].as<std::string>();
            oc.x    = o["x"].as<double>();
            oc.y    = o["y"].as<double>();
            if (oc.type == "circle") {
                oc.radius = o["radius"].as<double>();
            } else if (oc.type == "rectangle") {
                oc.width  = o["width"].as<double>();
                oc.height = o["height"].as<double>();
            } else {
                throw game::GameException(
                    "YAML: neznamy typ prekazky: " + oc.type);
            }
            cfg.obstacles.push_back(oc);
        }
    }

    // ── waste ──
    if (root["waste"]) {
        if (root["waste"]["radius_min"])
            cfg.waste.radius_min = root["waste"]["radius_min"].as<double>();
        if (root["waste"]["radius_max"])
            cfg.waste.radius_max = root["waste"]["radius_max"].as<double>();
        if (root["waste"]["types"]) {
            cfg.waste.types.clear();
            for (const auto& t : root["waste"]["types"]) {
                cfg.waste.types.push_back(t.as<std::string>());
            }
        }
    }

    // ── game ──
    if (root["game"]) {
        if (root["game"]["duration_sec"])
            cfg.game.duration_sec = root["game"]["duration_sec"].as<double>();
        if (root["game"]["initial_count"])
            cfg.game.initial_count = root["game"]["initial_count"].as<int>();
    }

    return cfg;
}

// ─────────────────────────────────────────────────────────────────────────────
// GameEnvironment implementacia
// ─────────────────────────────────────────────────────────────────────────────

namespace {
    Config toEnvConfig(const GameEnvironmentConfig& cfg) {
        Config c;
        c.map_filename = cfg.map_filename;
        c.resolution   = cfg.resolution;
        return c;
    }
}

GameEnvironment::GameEnvironment(const GameEnvironmentConfig& cfg)
    : Environment(toEnvConfig(cfg)),
      cfg_(cfg)
{
    // Postaviť prekážky cez Factory
    for (const auto& o : cfg.obstacles) {
        if (o.type == "circle") {
            obstacles_.push_back(
                std::make_unique<game::CircleObstacle>(
                    geometry::Point2d{o.x, o.y}, o.radius));
        } else if (o.type == "rectangle") {
            obstacles_.push_back(
                std::make_unique<game::RectangleObstacle>(
                    geometry::Point2d{o.x, o.y}, o.width, o.height));
        }
    }

    // Stanica
    station_ = std::make_unique<game::Station>(
        geometry::Point2d{cfg.station.x, cfg.station.y},
        cfg.station.radius);

    // Zakreslíme prekážky do precomputed occupancy gridu
    // (occupancy_ inicializuje Environment zo stien mapy)
    bakeObstaclesIntoGrid();
}

void GameEnvironment::bakeObstaclesIntoGrid() {
    const double res = getResolution();

    for (const auto& obs : obstacles_) {
        const auto* circle = dynamic_cast<const game::CircleObstacle*>(obs.get());
        const auto* rect   = dynamic_cast<const game::RectangleObstacle*>(obs.get());

        if (circle) {
            // Vyplníme kružnicu pixlami
            int cx, cy;
            worldToPixel(circle->getPosition().x, circle->getPosition().y, cx, cy);
            int r_px = static_cast<int>(std::ceil(circle->getRadius() / res));
            for (int dy = -r_px; dy <= r_px; ++dy) {
                for (int dx = -r_px; dx <= r_px; ++dx) {
                    if (dx*dx + dy*dy > r_px*r_px) continue;
                    int px = cx + dx, py = cy + dy;
                    if (px >= 0 && px < occupancy_.cols &&
                        py >= 0 && py < occupancy_.rows) {
                        occupancy_.at<uint8_t>(py, px) = 255;
                    }
                }
            }
        } else if (rect) {
            // Vyplníme obdĺžnik pixlami
            int x0, y0, x1, y1;
            worldToPixel(rect->getPosition().x, rect->getPosition().y, x0, y0);
            worldToPixel(rect->getPosition().x + rect->getWidth(),
                         rect->getPosition().y - rect->getHeight(), x1, y1);
            // Zabezpečíme správne poradie (worldToPixel flipuje y)
            if (x0 > x1) std::swap(x0, x1);
            if (y0 > y1) std::swap(y0, y1);
            cv::rectangle(occupancy_,
                          cv::Point(std::max(0, x0), std::max(0, y0)),
                          cv::Point(std::min(occupancy_.cols - 1, x1),
                                    std::min(occupancy_.rows - 1, y1)),
                          cv::Scalar(255), cv::FILLED);
        }
    }
}

bool GameEnvironment::isCollision(double x, double y, double radius) const {
    // 1) kolízia s mapou (steny)
    if (Environment::isCollision(x, y, radius)) {
        return true;
    }
    // 2) kolízia s prekážkami
    for (const auto& obs : obstacles_) {
        if (obs->collidesWith(x, y, radius)) {
            return true;
        }
    }
    return false;
}

bool GameEnvironment::isOccupied(double x, double y) const {
    // 1) stena na mape?
    if (Environment::isOccupied(x, y)) return true;
    // 2) prekážka v bode? (point-in-shape pre lidar)
    for (const auto& obs : obstacles_) {
        if (obs->collidesWith(x, y, 0.0)) {
            return true;
        }
    }
    return false;
}

bool GameEnvironment::isAtStation(double x, double y, double radius) const {
    return station_->collidesWith(x, y, radius);
}

} // namespace environment