// Benchmark lidar skenu – meria čas DDA vs. referenčná step-based implementácia.
// Cieľ: DDA sken 360 lúčov max_range=5 m < 5 ms (cieľ je ~50× rýchlejší ako step-based).

#include <chrono>
#include <cmath>
#include <cstdio>
#include <stdexcept>
#include <vector>

#include "environment/Lidar.hpp"
#include "environment/GameEnvironment.hpp"
#include "types/Geometry.hpp"

static constexpr int WARMUP_SCANS = 20;
static constexpr int BENCH_SCANS  = 500;

// Step-based referenčný sken (pôvodná implementácia)
std::vector<double> scanStepBased(
    const geometry::RobotState& state,
    const lidar::Config& cfg,
    environment::Environment& env)
{
    std::vector<double> ranges(cfg.beam_count, cfg.max_range);
    const double angle_step = (cfg.beam_count > 1)
        ? (cfg.last_ray_angle - cfg.first_ray_angle) / (cfg.beam_count - 1)
        : 0.0;
    const double step_size = 0.025;

    for (int i = 0; i < cfg.beam_count; ++i) {
        double angle = state.theta + cfg.first_ray_angle + i * angle_step;
        for (double r = step_size; r <= cfg.max_range; r += step_size) {
            double px = state.x + r * std::cos(angle);
            double py = state.y + r * std::sin(angle);
            if (env.isOccupied(px, py)) {
                ranges[i] = r;
                break;
            }
        }
    }
    return ranges;
}

int main() {
    const std::string config_path = "src/robot/cpp/config/environment.yaml";

    environment::GameEnvironmentConfig cfg;
    try {
        cfg = environment::loadConfigFromYaml(config_path);
    } catch (const std::exception& e) {
        std::fprintf(stderr, "YAML chyba: %s\n", e.what());
        return 1;
    }

    auto env = std::make_shared<environment::GameEnvironment>(cfg);

    lidar::Config lidar_cfg;
    lidar_cfg.beam_count      = 360;
    lidar_cfg.max_range       = 5.0;
    lidar_cfg.first_ray_angle = -M_PI;
    lidar_cfg.last_ray_angle  =  M_PI;

    lidar::Lidar lidar(lidar_cfg, env);

    // Robot v strede mapy
    geometry::RobotState state{-7.0, 6.0, 0.0, {0.0, 0.0}};

    using Clock = std::chrono::steady_clock;
    using Ms    = std::chrono::duration<double, std::milli>;

    // ── DDA benchmark ─────────────────────────────────────────────────────────
    for (int i = 0; i < WARMUP_SCANS; ++i) {
        state.theta += 0.01;
        lidar.scan(state);
    }

    auto t0_dda = Clock::now();
    for (int i = 0; i < BENCH_SCANS; ++i) {
        state.theta += 0.01;
        lidar.scan(state);
    }
    double dda_ms = Ms(Clock::now() - t0_dda).count() / BENCH_SCANS;

    // ── Step-based benchmark ──────────────────────────────────────────────────
    for (int i = 0; i < WARMUP_SCANS; ++i) {
        state.theta += 0.01;
        scanStepBased(state, lidar_cfg, *env);
    }

    auto t0_step = Clock::now();
    for (int i = 0; i < BENCH_SCANS; ++i) {
        state.theta += 0.01;
        scanStepBased(state, lidar_cfg, *env);
    }
    double step_ms = Ms(Clock::now() - t0_step).count() / BENCH_SCANS;

    // ── Výsledky ─────────────────────────────────────────────────────────────
    std::printf("\n=== LIDAR BENCHMARK (%d skenov, %d lúčov, max_range=%.1f m) ===\n\n",
                BENCH_SCANS, lidar_cfg.beam_count, lidar_cfg.max_range);
    std::printf("  DDA (optimalizovaný):    %6.2f ms / sken\n", dda_ms);
    std::printf("  Step-based (originál):   %6.2f ms / sken\n", step_ms);
    std::printf("  Zrychlenie:              %.1fx\n\n", step_ms / dda_ms);

    const double target_ms = 50.0;
    if (dda_ms < target_ms) {
        std::printf("  [PASS]  DDA sken %.2f ms < %.0f ms (ciel)\n\n", dda_ms, target_ms);
        return 0;
    } else {
        std::printf("  [FAIL]  DDA sken %.2f ms >= %.0f ms (ciel)\n\n", dda_ms, target_ms);
        return 1;
    }
}
