#include "environment/Environment.hpp"
#include <stdexcept>
#include <cmath>

namespace environment {

Environment::Environment(const Config& config) : resolution_(config.resolution) {
    map_ = cv::imread(config.map_filename, cv::IMREAD_GRAYSCALE);
    if (map_.empty()) {
        throw std::runtime_error(
            "Environment: nepodarilo sa načítať mapu: " + config.map_filename);
    }
    // Základný occupancy grid = invertovaná mapa (< 128 → obsadené → 255)
    occupancy_ = cv::Mat::zeros(map_.size(), CV_8UC1);
    for (int r = 0; r < map_.rows; ++r) {
        for (int c = 0; c < map_.cols; ++c) {
            if (map_.at<uint8_t>(r, c) < 128) {
                occupancy_.at<uint8_t>(r, c) = 255;
            }
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Konverzia svet (metre) → pixel.
// Stred mapy = (0, 0). Ľavý dolný roh mapy = (-W/2, -H/2).
// Pixel (0,0) je v PNG ľavý HORNÝ roh, takže Y musíme flipnúť.
// ─────────────────────────────────────────────────────────────────────────────
void Environment::worldToPixel(double x, double y, int& px, int& py) const {
    const double w_m = map_.cols * resolution_;
    const double h_m = map_.rows * resolution_;
    px = static_cast<int>((x + w_m / 2.0) / resolution_);
    py = static_cast<int>((h_m / 2.0 - y) / resolution_);
}

// ─────────────────────────────────────────────────────────────────────────────
// isOccupied – pre LIDAR. Jeden pixel.
// ─────────────────────────────────────────────────────────────────────────────
bool Environment::isOccupied(double x, double y) const {
    int px, py;
    worldToPixel(x, y, px, py);

    // Mimo mapy = stena (lidar uvidí hranice).
    if (px < 0 || px >= map_.cols || py < 0 || py >= map_.rows) {
        return true;
    }
    return map_.at<uint8_t>(py, px) < 128;
}

// ─────────────────────────────────────────────────────────────────────────────
// isCollision – pre kolíziu robota. Bod + polomer.
// ─────────────────────────────────────────────────────────────────────────────
bool Environment::isCollision(double x, double y, double radius) const {
    int cx, cy;
    worldToPixel(x, y, cx, cy);
    const int r = static_cast<int>(std::ceil(radius / resolution_));

    for (int dy = -r; dy <= r; ++dy) {
        for (int dx = -r; dx <= r; ++dx) {
            if (dx*dx + dy*dy > r*r) continue;

            int px = cx + dx;
            int py = cy + dy;

            if (px < 0 || px >= map_.cols ||
                py < 0 || py >= map_.rows) {
                return true;
            }
            if (map_.at<uint8_t>(py, px) < 128) {
                return true;
            }
        }
    }
    return false;
}

} // namespace environment