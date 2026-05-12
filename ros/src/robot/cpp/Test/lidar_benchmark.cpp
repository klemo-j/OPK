#include <chrono>
#include <cmath>
#include <gtest/gtest.h>

#include "environment/GameEnvironment.hpp"
#include "environment/Lidar.hpp"
#include "types/Geometry.hpp"

// ── Spoločná fixtures ─────────────────────────────────────────────────────────

class LidarTest : public ::testing::Test {
protected:
    void SetUp() override {
        auto cfg = environment::loadConfigFromYaml(
            "src/robot/cpp/config/environment.yaml");
        env_ = std::make_shared<environment::GameEnvironment>(cfg);

        lidar_cfg_.beam_count      = 360;
        lidar_cfg_.max_range       = 5.0;
        lidar_cfg_.first_ray_angle = -M_PI;
        lidar_cfg_.last_ray_angle  =  M_PI;

        lidar_ = std::make_unique<lidar::Lidar>(lidar_cfg_, env_);
        state_ = geometry::RobotState{-7.0, 6.0, 0.0, {0.0, 0.0}};
    }

    std::shared_ptr<environment::GameEnvironment> env_;
    lidar::Config lidar_cfg_;
    std::unique_ptr<lidar::Lidar> lidar_;
    geometry::RobotState state_;

    // Zmeria priemerný čas jedného skenu cez N opakovaní
    double avgScanMs(int n) {
        using Clock = std::chrono::steady_clock;
        using Ms    = std::chrono::duration<double, std::milli>;

        // Warmup – vyhrejeme cache
        for (int i = 0; i < 10; ++i) {
            state_.theta += 0.01;
            lidar_->scan(state_);
        }

        auto t0 = Clock::now();
        for (int i = 0; i < n; ++i) {
            state_.theta += 0.01;
            lidar_->scan(state_);
        }
        return Ms(Clock::now() - t0).count() / n;
    }
};

// ── Testy ─────────────────────────────────────────────────────────────────────

// Jeden sken musí trvať menej ako 50 ms
TEST_F(LidarTest, SkenTrvaMaximalne50ms) {
    double ms = avgScanMs(100);
    EXPECT_LT(ms, 50.0)
        << "Priemerny cas skenu: " << ms << " ms (limit: 50 ms)";
}

// Výstup má správny počet lúčov
TEST_F(LidarTest, VrataSprávnyPocetLucov) {
    auto ranges = lidar_->scan(state_);
    EXPECT_EQ(static_cast<int>(ranges.size()), lidar_cfg_.beam_count);
}

// Každá vzdialenosť je v rozsahu [0, max_range]
TEST_F(LidarTest, VzdialenostiSuVRozsahu) {
    auto ranges = lidar_->scan(state_);
    for (int i = 0; i < static_cast<int>(ranges.size()); ++i) {
        EXPECT_GE(ranges[i], 0.0)         << "Luc " << i << " je zaporny";
        EXPECT_LE(ranges[i], lidar_cfg_.max_range) << "Luc " << i << " prekracuje max_range";
    }
}

// Nie všetky lúče môžu byť max_range – robot je obklopený stenami
TEST_F(LidarTest, NieVsetkyLuceMaxRange) {
    auto ranges = lidar_->scan(state_);
    int hits = 0;
    for (double r : ranges) {
        if (r < lidar_cfg_.max_range) ++hits;
    }
    EXPECT_GT(hits, 0) << "Ziaden luc netrafil stenu – lidar nejde";
}

// ── Main ──────────────────────────────────────────────────────────────────────

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
