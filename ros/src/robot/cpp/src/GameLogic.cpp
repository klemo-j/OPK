#include "game/GameLogic.hpp"

namespace game {

GameLogic::GameLogic(std::shared_ptr<environment::GameEnvironment> env,
                     const waste::WasteGeneratorConfig& gen_cfg,
                     int max_capacity,
                     double game_duration_sec,
                     unsigned int seed)
    : env_(env),
      generator_(gen_cfg, seed),
      max_capacity_(max_capacity),
      game_duration_(game_duration_sec),
      rng_(seed)
{
    state_.p1.capacity_max = max_capacity;
    state_.p2.capacity_max = max_capacity;
    state_.time_left = game_duration_sec;
}

void GameLogic::reset() {
    wastes_.clear();
    p1_in_robot_.clear();
    p2_in_robot_.clear();
    state_ = GameState{};
    state_.p1.capacity_max = max_capacity_;
    state_.p2.capacity_max = max_capacity_;
    state_.time_left = game_duration_;
}

bool GameLogic::findFreeSpot(double radius, double& out_x, double& out_y,
                             int max_attempts)
{
    const double w = env_->getWidth();
    const double h = env_->getHeight();
    const double margin = radius + 0.2;

    std::uniform_real_distribution<double> x_dist(-w/2.0 + margin,  w/2.0 - margin);
    std::uniform_real_distribution<double> y_dist(-h/2.0 + margin,  h/2.0 - margin);

    for (int i = 0; i < max_attempts; ++i) {
        double x = x_dist(rng_);
        double y = y_dist(rng_);
        if (!env_->isCollision(x, y, radius)) {
            out_x = x;
            out_y = y;
            return true;
        }
    }
    return false;
}

void GameLogic::spawnWastes(int count, int max_attempts_per_waste) {
    for (int i = 0; i < count; ++i) {
        auto w = generator_.generateRandom();
        const double r = w->getRadius();

        double x, y;
        if (findFreeSpot(r, x, y, max_attempts_per_waste)) {
            auto fresh = generator_.generateOfType(w->getType(), {x, y}, r);
            wastes_.push_back(std::move(fresh));
        }
    }
    state_.waste_in_world = static_cast<int>(wastes_.size());
}

// Spoločný handler pre jedného hráča – zber + vyloženie
void GameLogic::handlePlayer(double rx, double ry, double rr,
                             PlayerState& player,
                             std::vector<waste::WasteType>& in_robot,
                             bool& changed)
{
    // ── Zber odpadkov ─────────────────────────────────────────────────────
    // Kapacita je v slotoch (PAPER=1, PLASTIC=2, GLASS=3).
    // Ak sa odpadok nezmestí, preskočíme ho (continue) – možno sa zmestí menší.
    for (auto it = wastes_.begin(); it != wastes_.end(); ) {
        auto& w = *it;
        const int slots_needed = w->getSlots();
        if (player.capacity_used + slots_needed > max_capacity_) {
            ++it;
            continue;  // odpadok sa nezmestí, skúsime ďalší
        }
        if (w->collidesWith(rx, ry, rr)) {
            in_robot.push_back(w->getType());
            player.capacity_used += slots_needed;
            it = wastes_.erase(it);
            changed = true;
        } else {
            ++it;
        }
    }

    // ── Vyloženie v stanici ──────────────────────────────────────────────
    bool in_station = env_->isAtStation(rx, ry, rr);
    if (in_station && !player.was_in_station && !in_robot.empty()) {
        for (const auto& type : in_robot) {
            player.collected_counts[type]++;
            // score_value == slots == 1/2/3
            switch (type) {
                case waste::WasteType::PAPER:   player.score += 1; break;
                case waste::WasteType::PLASTIC: player.score += 2; break;
                case waste::WasteType::GLASS:   player.score += 3; break;
            }
        }
        in_robot.clear();
        player.capacity_used = 0;  // reset slotov
        changed = true;
    }
    player.was_in_station = in_station;
}

void GameLogic::determineWinner() {
    if (state_.p1.score > state_.p2.score) {
        state_.winner = "p1";
    } else if (state_.p2.score > state_.p1.score) {
        state_.winner = "p2";
    } else {
        state_.winner = "draw";
    }
}

bool GameLogic::update(double p1_x, double p1_y,
                       double p2_x, double p2_y,
                       double rr,
                       double dt)
{
    if (state_.game_over) return false;

    bool changed = false;

    // ── Časomiera ─────────────────────────────────────────────────────────
    state_.time_left -= dt;
    if (state_.time_left <= 0.0) {
        state_.time_left = 0.0;
        state_.game_over = true;
        determineWinner();
        changed = true;
    }

    // Ak hra skončila po tomto ticku, ešte stále spracujeme zber (fér)
    handlePlayer(p1_x, p1_y, rr, state_.p1, p1_in_robot_, changed);
    handlePlayer(p2_x, p2_y, rr, state_.p2, p2_in_robot_, changed);

    state_.waste_in_world = static_cast<int>(wastes_.size());
    return changed || state_.game_over;
}

} // namespace game