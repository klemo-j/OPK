#include <chrono>
#include <cmath>
#include <gtest/gtest.h>

#include "environment/GameEnvironment.hpp"
#include "environment/Lidar.hpp"

TEST(LidarTest, SkenTrvaMaximalne50ms) {
    auto cfg = environment::loadConfigFromYaml("src/robot/cpp/config/environment.yaml");
    auto env = std::make_shared<environment::GameEnvironment>(cfg);

    lidar::Config lidar_cfg;
    lidar_cfg.beam_count      = 360;
    lidar_cfg.max_range       = 5.0;
    lidar_cfg.first_ray_angle = -M_PI;
    lidar_cfg.last_ray_angle  =  M_PI;

    lidar::Lidar lidar(lidar_cfg, env);
    geometry::RobotState state{-7.0, 6.0, 0.0, {0.0, 0.0}};

    auto t0 = std::chrono::steady_clock::now();
    lidar.scan(state);
    double ms = std::chrono::duration<double, std::milli>(
        std::chrono::steady_clock::now() - t0).count();

    EXPECT_LT(ms, 50.0) << "Sken trval " << ms << " ms, limit je 50 ms";
}
