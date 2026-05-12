#pragma once

#include "waste/WasteFactory.hpp"
#include "environment/GameEnvironment.hpp"
#include "types/Geometry.hpp"

#include <vector>
#include <memory>
#include <map>
#include <random>
#include <string>

namespace game {

// ─────────────────────────────────────────────────────────────────────────────
// Stav jedného hráča
// ─────────────────────────────────────────────────────────────────────────────
struct PlayerState {
    int score = 0;
    int capacity_used = 0;
    int capacity_max = 5;
    std::map<waste::WasteType, int> collected_counts;
    bool was_in_station = false;     // edge detection
};

// ─────────────────────────────────────────────────────────────────────────────
// Spoločný stav hry – pre publikovanie do ROS
// ─────────────────────────────────────────────────────────────────────────────
struct GameState {
    PlayerState p1;
    PlayerState p2;
    int waste_in_world = 0;
    double time_left = 0.0;
    bool game_over = false;
    std::string winner;       // "p1", "p2", "draw", "" (počas hry)
};

// ─────────────────────────────────────────────────────────────────────────────
// GameLogic – čistá herná logika BEZ ROS závislosti.
// Podporuje 2 hráčov, ktorí zbierajú zo spoločného poolu odpadkov.
// ─────────────────────────────────────────────────────────────────────────────
class GameLogic {
public:
    GameLogic(std::shared_ptr<environment::GameEnvironment> env,
              const waste::WasteGeneratorConfig& gen_cfg,
              int max_capacity,
              double game_duration_sec,
              unsigned int seed = 42);

    // Vygeneruje N nových odpadkov a pridá ich do sveta
    void spawnWastes(int count, int max_attempts_per_waste = 50);

    // Aktualizácia hry. Volá sa periodicky.
    // Sleduje oboch hráčov, rieši zber + vyloženie, počíta čas, určí víťaza.
    // Vracia true ak sa stav zmenil.
    bool update(double p1_x, double p1_y,
                double p2_x, double p2_y,
                double robot_radius,
                double dt);

    // Read-only
    const std::vector<std::unique_ptr<waste::Waste>>& getActiveWastes() const {
        return wastes_;
    }
    const GameState& getState() const { return state_; }

    // Reset hry (pre service /game/reset)
    void reset();

private:
    void handlePlayer(double rx, double ry, double rr,
                      PlayerState& player,
                      std::vector<waste::WasteType>& in_robot,
                      bool& changed);

    bool findFreeSpot(double radius, double& out_x, double& out_y,
                      int max_attempts);

    void determineWinner();

    std::shared_ptr<environment::GameEnvironment> env_;
    waste::WasteGenerator generator_;
    int max_capacity_;
    double game_duration_;

    std::vector<std::unique_ptr<waste::Waste>> wastes_;
    std::vector<waste::WasteType> p1_in_robot_;
    std::vector<waste::WasteType> p2_in_robot_;

    GameState state_;

    std::mt19937 rng_;
};

} // namespace game