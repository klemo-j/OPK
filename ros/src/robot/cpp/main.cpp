#include <iostream>
#include <vector>
#include <memory>
#include <cmath>
#include <opencv2/opencv.hpp>

#include "../include/environment/Environment.h"
#include "../include/environment/Lidar.h"
#include "../include/robot/Robot.h"
#include "../include/types/Geometry.h"

class Canvas {
public:
    Canvas(const std::string& map_filename, double resolution)
        : resolution_(resolution)
    {
        map_img_ = cv::imread(map_filename, cv::IMREAD_COLOR);
        if (map_img_.empty()) {
            std::cerr << "Chyba: Nepodarilo sa nacitat mapu.\n";
        }
    }

    void draw(const geometry::RobotState& state,
              const std::vector<geometry::Point2d>& scan)
    {
        if (map_img_.empty()) return;

        cv::Mat display = map_img_.clone();

        cv::Point robot_px(
            static_cast<int>(state.x / resolution_),
            static_cast<int>(state.y / resolution_)
        );

        cv::circle(display, robot_px, 5, cv::Scalar(0, 255, 0), -1);

        int line_length = 20;
        cv::Point heading_px(
            static_cast<int>(robot_px.x + line_length * std::cos(state.theta)),
            static_cast<int>(robot_px.y + line_length * std::sin(state.theta))
        );

        cv::line(display, robot_px, heading_px, cv::Scalar(255, 0, 0), 2);

        for (const auto& pt : scan) {
            cv::circle(
                display,
                cv::Point(
                    static_cast<int>(pt.x / resolution_),
                    static_cast<int>(pt.y / resolution_)
                ),
                2,
                cv::Scalar(0, 0, 255),
                -1
            );
        }

        cv::imshow("Robot Simulacia", display);
    }

private:
    cv::Mat map_img_;
    double resolution_;
};

int main()
{
    double res = 0.05;
    std::string map_path = "../resources/opk-map.png";

    auto env = std::make_shared<environment::Environment>(
        environment::Config{map_path, res}
    );

    lidar::Config lidar_cfg{5.0, 100, 0.0, 2 * M_PI};
    lidar::Lidar lidar(lidar_cfg, env);

    robot::Config robot_cfg{
        geometry::Twist{0.5, 1.0},
        geometry::Twist{1.0, 2.0},
        0.5,
        30
    };

    auto collision_cb = [env](geometry::RobotState state) {
        return env->isOccupied(state.x, state.y);
    };

    robot::Robot my_robot(robot_cfg, collision_cb);

    Canvas canvas(map_path, res);
    cv::namedWindow("Robot Simulacia");

    std::cout << "Ovladanie: W/S/A/D, ESC pre ukoncenie.\n";

    while (true) {
        int key = cv::waitKey(30);

        if (key == 27) {
            break;
        }

        if (key == 'w' || key == 'W') {
            my_robot.setVelocity({1.0, 0.0});
        } else if (key == 's' || key == 'S') {
            my_robot.setVelocity({-1.0, 0.0});
        } else if (key == 'a' || key == 'A') {
            my_robot.setVelocity({0.0, -1.0});
        } else if (key == 'd' || key == 'D') {
            my_robot.setVelocity({0.0, 1.0});
        }

        geometry::RobotState state = my_robot.getState();
        std::vector<geometry::Point2d> scan = lidar.scan(state);

        canvas.draw(state, scan);
    }

    return 0;
}