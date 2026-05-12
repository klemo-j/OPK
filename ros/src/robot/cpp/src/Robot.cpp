#include "../include/robot/Robot.hpp"
#include "../include/types/Geometry.hpp"

#include <cmath>
#include <chrono>
#include <thread>

namespace robot {

    Robot::Robot(const Config& config, const CollisionCb& collision_cb)
        : config_(config),
          state_{config.start_x, config.start_y, config.start_theta, {0.0, 0.0}},
          target_velocity_{0.0, 0.0},
          collision_cb_(collision_cb),
          running_flag_(true),
          in_collision_(false),
          last_command_time_(std::chrono::steady_clock::now())
    {
        worker_thread_ = std::thread(&Robot::loop, this);
    }

    Robot::~Robot()
    {
        running_flag_ = false;
        if (worker_thread_.joinable()) {
            worker_thread_.join();
        }
    }

    void Robot::setVelocity(const geometry::Twist& velocity) {
        std::unique_lock<std::mutex> lock(state_mutex_);
        target_velocity_ = velocity;
        last_command_time_ = std::chrono::steady_clock::now();

        // DEBUG
        fprintf(stderr, "[Robot::setVelocity] lin=%.2f ang=%.2f cmd_dur=%.2f\n",
                velocity.linear, velocity.angular, config_.command_duration);
    }

    geometry::RobotState Robot::getState() const {
        std::unique_lock<std::mutex> lock(state_mutex_);
        return state_;
    }

    bool Robot::isInCollision() const {
        return in_collision_;
    }

    void Robot::stepPhysics(const geometry::Twist& velocity, double dt)
    {
        double linear_error  = velocity.linear  - state_.velocity.linear;
        double angular_error = velocity.angular - state_.velocity.angular;

        double max_linear_change  = config_.accelerations.linear  * dt;
        double max_angular_change = config_.accelerations.angular * dt;

        if (velocity.linear == 0.0 && velocity.angular == 0.0) {
            max_linear_change  = config_.emergency_decelerations.linear  * dt;
            max_angular_change = config_.emergency_decelerations.angular * dt;
        }

        if      (linear_error >  max_linear_change) state_.velocity.linear += max_linear_change;
        else if (linear_error < -max_linear_change) state_.velocity.linear -= max_linear_change;
        else                                         state_.velocity.linear  = velocity.linear;

        if      (angular_error >  max_angular_change) state_.velocity.angular += max_angular_change;
        else if (angular_error < -max_angular_change) state_.velocity.angular -= max_angular_change;
        else                                           state_.velocity.angular  = velocity.angular;

        state_.theta += state_.velocity.angular * dt;
        state_.x     += state_.velocity.linear  * dt * std::cos(state_.theta);
        state_.y     += state_.velocity.linear  * dt * std::sin(state_.theta);
    }

    void Robot::loop()
    {
        while (running_flag_) {
            const double dt = static_cast<double>(config_.simulation_period_ms) / 1000.0;

            {
                std::unique_lock<std::mutex> lock(state_mutex_);

                auto now_t   = std::chrono::steady_clock::now();
                double elapsed = std::chrono::duration<double>(now_t - last_command_time_).count();
                geometry::Twist velocity = (elapsed <= config_.command_duration)
                    ? target_velocity_ : geometry::Twist{0.0, 0.0};

                geometry::RobotState old_state = state_;
                stepPhysics(velocity, dt);

                // Kolízia sa kontroluje a revertuje ATOMICKY so simulačným krokom.
                // getState() nikdy neuvidí pozíciu "vo vnútri steny".
                if (collision_cb_ && collision_cb_(state_)) {
                    state_        = old_state;
                    state_.velocity.linear  = 0.0;
                    state_.velocity.angular = 0.0;
                    in_collision_ = true;
                } else {
                    in_collision_ = false;
                }
            }

            std::this_thread::sleep_for(
                std::chrono::milliseconds(config_.simulation_period_ms));
        }
    }

    void Robot::update(const geometry::Twist& velocity, double dt)
    {
        std::unique_lock<std::mutex> lock(state_mutex_);
        stepPhysics(velocity, dt);
    }

} // namespace robot