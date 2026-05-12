#pragma once

#include <functional>
#include "types/Geometry.h"
#include <mutex>
#include <thread>
#include <atomic>
#include <chrono>

namespace robot {

    struct Config {
        geometry::Twist accelerations;
        geometry::Twist emergency_decelerations;
        double command_duration;
        int simulation_period_ms;
    };


    class Robot {
    public:
        using CollisionCb = std::function<bool(geometry::RobotState)>;

        Robot(const Config& config, const CollisionCb& collision_cb = nullptr);
        ~Robot();
        void setVelocity(const geometry::Twist& velocity);
        geometry::RobotState getState() const;
        bool isInCollision() const;


    protected:
        void update(const geometry::Twist& velocity, double dt);
    
    private:
        void loop();

        Config config_;
        geometry::RobotState state_;
        geometry::Twist target_velocity_;

        CollisionCb collision_cb_;

        std::atomic<bool> running_flag_;
        std::atomic<bool> in_collision_;

        std::thread worker_thread_;
        mutable std::mutex state_mutex_;

        std::chrono::steady_clock::time_point last_command_time_;

    };
} // namespace robot

