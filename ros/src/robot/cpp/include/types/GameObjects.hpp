#pragma once

#include "Geometry.hpp"
#include <string>
#include <stdexcept>
#include <cmath>
namespace game {

// ─────────────────────────────────────────────────────────────────────────────
// Vlastná výnimka (spĺňa požiadavku: std::exception)
// ─────────────────────────────────────────────────────────────────────────────
class GameException : public std::exception {
public:
    explicit GameException(const std::string& msg) : msg_("GameException: " + msg) {}
    const char* what() const noexcept override { return msg_.c_str(); }
private:
    std::string msg_;
};

// ─────────────────────────────────────────────────────────────────────────────
// Abstraktná základná trieda pre všetky herné objekty
// ─────────────────────────────────────────────────────────────────────────────
class GameObject {
public:
    explicit GameObject(const geometry::Point2d& position)
        : position_(position) {}

    virtual ~GameObject() = default;

    // Čisto virtuálna – každý objekt musí vedieť, či bod je v kolízii s ním
    virtual bool collidesWith(double x, double y, double robot_radius) const = 0;

    // Čisto virtuálna – meno/typ objektu pre debug/vizualizáciu
    virtual std::string getTypeName() const = 0;

    const geometry::Point2d& getPosition() const { return position_; }

protected:
    geometry::Point2d position_;
};

// ─────────────────────────────────────────────────────────────────────────────
// Prekážka – kruh (červený objekt)
// ─────────────────────────────────────────────────────────────────────────────
class CircleObstacle : public GameObject {
public:
    CircleObstacle(const geometry::Point2d& center, double radius)
        : GameObject(center), radius_(radius)
    {
        if (radius <= 0.0) {
            throw GameException("CircleObstacle: polomer musí byť kladný");
        }
    }

    bool collidesWith(double x, double y, double robot_radius) const override {
        double dx = x - position_.x;
        double dy = y - position_.y;
        double dist = std::sqrt(dx * dx + dy * dy);
        return dist < (radius_ + robot_radius);
    }

    std::string getTypeName() const override { return "CircleObstacle"; }
    double getRadius() const { return radius_; }

private:
    double radius_;
};

// ─────────────────────────────────────────────────────────────────────────────
// Prekážka – obdĺžnik (červený objekt)
// ─────────────────────────────────────────────────────────────────────────────
class RectangleObstacle : public GameObject {
public:
    // position = ľavý dolný roh
    RectangleObstacle(const geometry::Point2d& position, double width, double height)
        : GameObject(position), width_(width), height_(height)
    {
        if (width <= 0.0 || height <= 0.0) {
            throw GameException("RectangleObstacle: rozmery musia byť kladné");
        }
    }

    bool collidesWith(double x, double y, double robot_radius) const override {
        // Najbližší bod obdĺžnika k stredu robota
        double nearest_x = std::max(position_.x, std::min(x, position_.x + width_));
        double nearest_y = std::max(position_.y, std::min(y, position_.y + height_));
        double dx = x - nearest_x;
        double dy = y - nearest_y;
        return std::sqrt(dx * dx + dy * dy) < robot_radius;
    }

    std::string getTypeName() const override { return "RectangleObstacle"; }
    double getWidth()  const { return width_; }
    double getHeight() const { return height_; }

private:
    double width_;
    double height_;
};

// ─────────────────────────────────────────────────────────────────────────────
// Stanica – kde robot vykladá odpad (zelený objekt)
// ─────────────────────────────────────────────────────────────────────────────
class Station : public GameObject {
public:
    Station(const geometry::Point2d& position, double radius)
        : GameObject(position), radius_(radius)
    {
        if (radius <= 0.0) {
            throw GameException("Station: polomer musí byť kladný");
        }
    }

    bool collidesWith(double x, double y, double robot_radius) const override {
        double dx = x - position_.x;
        double dy = y - position_.y;
        return std::sqrt(dx * dx + dy * dy) < (radius_ + robot_radius);
    }

    // Robot je v stanici, keď jeho stred je v polomere stanice
    bool isRobotInside(double x, double y, double robot_radius) const {
        double dx = x - position_.x;
        double dy = y - position_.y;
        return std::sqrt(dx * dx + dy * dy) < (radius_ + robot_radius);
    }

    std::string getTypeName() const override { return "Station"; }
    double getRadius() const { return radius_; }

private:
    double radius_;
};

} // namespace game
