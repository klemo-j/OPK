#include <gtest/gtest.h>
#include "../include/environment/Environment.h"
#include "../include/robot/Robot.h"
#include <cmath>
#include <thread>
#include <chrono>

TEST(EnvironmentTest, MimoMapyJePrekazka) {

    environment::Config cfg{"../../resources/opk-map.png", 0.05};
    environment::Environment env(cfg);


    EXPECT_TRUE(env.isOccupied(-10.0, -10.0));
}

class TestRobot : public robot::Robot {
public:
    using robot::Robot::Robot;
    using robot::Robot::update;
};

static robot::Config makeRobotConfig() {
    robot::Config cfg;
    cfg.accelerations = {1.0, 1.0};
    cfg.emergency_decelerations = {2.0, 2.0};
    cfg.command_duration = 0.1;
    cfg.simulation_period_ms = 10;
    return cfg;
}

TEST(RobotTest, PoSpusteniStoji) {
    TestRobot robot(makeRobotConfig());
    geometry::RobotState state = robot.getState();

    EXPECT_DOUBLE_EQ(state.x, 10.0);
    EXPECT_DOUBLE_EQ(state.y, 10.0);
    EXPECT_DOUBLE_EQ(state.theta, 0.0);
    EXPECT_DOUBLE_EQ(state.velocity.linear, 0.0);
    EXPECT_DOUBLE_EQ(state.velocity.angular, 0.0);
}

TEST(RobotTest, LinearnaRychlostSaZvysujePostupne) {
    TestRobot robot(makeRobotConfig());

    robot.update({1.0, 0.0}, 0.1);
    geometry::RobotState state = robot.getState();

    EXPECT_NEAR(state.velocity.linear, 0.1, 1e-9);
    EXPECT_NEAR(state.velocity.angular, 0.0, 1e-9);
}

TEST(RobotTest, UhlovaRychlostSaZvysujePostupne) {
    TestRobot robot(makeRobotConfig());

    robot.update({0.0, 1.0}, 0.1);
    geometry::RobotState state = robot.getState();

    EXPECT_NEAR(state.velocity.linear, 0.0, 1e-9);
    EXPECT_NEAR(state.velocity.angular, 0.1, 1e-9);
}

TEST(RobotTest, PriThetaNulaSaPohybujePoX) {
    TestRobot robot(makeRobotConfig());

    robot.update({1.0, 0.0}, 1.0);
    geometry::RobotState state = robot.getState();

    EXPECT_GT(state.x, 0.0);
    EXPECT_NEAR(state.y, 0.0, 1e-9);
}

TEST(RobotTest, RotaciaMeniTheta) {
    TestRobot robot(makeRobotConfig());

    robot.update({0.0, 1.0}, 1.0);
    geometry::RobotState state = robot.getState();

    EXPECT_GT(state.theta, 0.0);
}

TEST(RobotTest, NeprekrociCielovuRychlost) {
    TestRobot robot(makeRobotConfig());

    robot.update({0.05, 0.0}, 1.0);
    geometry::RobotState state = robot.getState();

    EXPECT_NEAR(state.velocity.linear, 0.05, 1e-9);
    EXPECT_NEAR(state.velocity.angular, 0.0, 1e-9);
}

TEST(RobotTest, SpomalujeKNule) {
    TestRobot robot(makeRobotConfig());

    robot.update({1.0, 0.0}, 1.0);
    geometry::RobotState state1 = robot.getState();

    robot.update({0.0, 0.0}, 0.1);
    geometry::RobotState state2 = robot.getState();

    EXPECT_LT(state2.velocity.linear, state1.velocity.linear);
}

TEST(RobotTest, PoTimeouteZacneZastavovat) {
    robot::Config cfg = makeRobotConfig();
    cfg.command_duration = 0.1;
    cfg.simulation_period_ms = 10;

    robot::Robot robot(cfg);

    robot.setVelocity({1.0, 0.0});
    std::this_thread::sleep_for(std::chrono::milliseconds(80));
    geometry::RobotState movingState = robot.getState();

    EXPECT_GT(movingState.velocity.linear, 0.0);

    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    geometry::RobotState slowedState = robot.getState();

    EXPECT_LT(slowedState.velocity.linear, movingState.velocity.linear);
}

