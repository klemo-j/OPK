#pragma once

#include "types/GameObjects.hpp"
#include "types/Geometry.hpp"
#include <string>
#include <cmath>

namespace waste {

// ─────────────────────────────────────────────────────────────────────────────
// Typy odpadkov
// ─────────────────────────────────────────────────────────────────────────────
enum class WasteType {
    PAPER,
    PLASTIC,
    GLASS
};

inline std::string wasteTypeToString(WasteType type) {
    switch (type) {
        case WasteType::PAPER:   return "paper";
        case WasteType::PLASTIC: return "plastic";
        case WasteType::GLASS:   return "glass";
        default:                 return "unknown";
    }
}

inline WasteType wasteTypeFromString(const std::string& s) {
    if (s == "paper")   return WasteType::PAPER;
    if (s == "plastic") return WasteType::PLASTIC;
    if (s == "glass")   return WasteType::GLASS;
    throw game::GameException("Neznámy typ odpadu: " + s);
}

// ─────────────────────────────────────────────────────────────────────────────
// Abstraktná základná trieda Waste
// Dedí od GameObject – odpad je herný objekt so svojou polohou a polomerom
// ─────────────────────────────────────────────────────────────────────────────
class Waste : public game::GameObject {
public:
    Waste(const geometry::Point2d& position, double radius, WasteType type, int score_value)
        : game::GameObject(position),
          radius_(radius),
          type_(type),
          score_value_(score_value),
          collected_(false)
    {
        if (radius <= 0.0) {
            throw game::GameException("Waste: polomer musí byť kladný");
        }
    }

    virtual ~Waste() = default;

    // Robot zbiera odpad, ak je jeho stred blízko odpadku
    bool collidesWith(double x, double y, double robot_radius) const override {
        if (collected_) return false;
        double dx = x - position_.x;
        double dy = y - position_.y;
        return std::sqrt(dx * dx + dy * dy) < (radius_ + robot_radius);
    }

    std::string getTypeName() const override {
        return "Waste_" + wasteTypeToString(type_);
    }

    // Čisto virtuálna – každý typ odpadu má vlastný popis
    virtual std::string getDescription() const = 0;

    WasteType getType()       const { return type_; }
    double    getRadius()     const { return radius_; }
    int       getScoreValue() const { return score_value_; }
    int       getSlots()      const { return score_value_; }  // 1/2/3 bodov = 1/2/3 miest v batohu
    bool      isCollected()   const { return collected_; }

    void collect() {
        if (collected_) {
            throw game::GameException("Odpad bol už zozbieraný");
        }
        collected_ = true;
    }

protected:
    double    radius_;
    WasteType type_;
    int       score_value_;
    bool      collected_;
};

// ─────────────────────────────────────────────────────────────────────────────
// Konkrétne typy odpadkov
// ─────────────────────────────────────────────────────────────────────────────

// Fixné rozmery podľa typu (viditeľne rozlíšiteľné)
static constexpr double PAPER_RADIUS   = 0.12;   // malý  – 1 miesto, 1 bod
static constexpr double PLASTIC_RADIUS = 0.18;   // stredný – 2 miesta, 2 body
static constexpr double GLASS_RADIUS   = 0.25;   // veľký – 3 miesta, 3 body

class PaperWaste : public Waste {
public:
    explicit PaperWaste(const geometry::Point2d& position, double /*ignored*/ = 0)
        : Waste(position, PAPER_RADIUS, WasteType::PAPER, /*score=*/1) {}

    std::string getDescription() const override {
        return "Papier – malý, 1 miesto v batohu, 1 bod";
    }
};

class PlasticWaste : public Waste {
public:
    explicit PlasticWaste(const geometry::Point2d& position, double /*ignored*/ = 0)
        : Waste(position, PLASTIC_RADIUS, WasteType::PLASTIC, /*score=*/2) {}

    std::string getDescription() const override {
        return "Plast – stredný, 2 miesta v batohu, 2 body";
    }
};

class GlassWaste : public Waste {
public:
    explicit GlassWaste(const geometry::Point2d& position, double /*ignored*/ = 0)
        : Waste(position, GLASS_RADIUS, WasteType::GLASS, /*score=*/3) {}

    std::string getDescription() const override {
        return "Sklo – veľký, 3 miesta v batohu, 3 body";
    }
};

} // namespace waste
