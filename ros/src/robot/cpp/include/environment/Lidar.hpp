#pragma once

#include "../types/Geometry.hpp"
#include "Environment.hpp"
#include <vector>
#include <memory>

namespace lidar {

    struct Config {
        double max_range;
        int beam_count;
        double first_ray_angle;
        double last_ray_angle;
    };

    class Lidar {
    public:
        Lidar(const Config& config, std::shared_ptr<environment::Environment> env);
        // Returns one range per beam (max_range if no hit).
        std::vector<double> scan(const geometry::RobotState& state) const;


    private:
        Config config_;
        std::shared_ptr<environment::Environment> env_;
    };

} // namespace lidar