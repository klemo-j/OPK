#pragma once

#include <opencv2/opencv.hpp>
#include <string>

namespace environment {

    struct Config {
        std::string map_filename;
        double resolution;     // metre na pixel
    };

    class Environment {
    public:
        explicit Environment(const Config& config);
        virtual ~Environment() = default;

        // ── Pre LIDAR ────────────────────────────────────────────────────────
        // Vráti true ak v bode (x,y) [meter] je stena podľa PNG mapy.
        virtual bool isOccupied(double x, double y) const;

        // ── Pre KOLÍZIE ROBOTA ──────────────────────────────────────────────
        virtual bool isCollision(double x, double y, double radius) const;

        // ── Pre vizualizáciu ────────────────────────────────────────────────
        int            getMapWidthPx()  const { return map_.cols; }
        int            getMapHeightPx() const { return map_.rows; }
        double         getResolution()  const { return resolution_; }
        const cv::Mat& getMapImage()    const { return map_; }

        // V metroch
        double getWidth()  const { return map_.cols * resolution_; }
        double getHeight() const { return map_.rows * resolution_; }

        // Priamy prístup na occupancy grid pre rýchly DDA lidar
        const cv::Mat& getOccupancyGrid() const { return occupancy_; }

    protected:
        void worldToPixel(double x, double y, int& px, int& py) const;

        // Precomputed occupancy grid (mapa + všetky prekážky): 0=volné, >0=obsadené
        // Subklasy ho môžu rozšíriť (napr. nakresliť geometrické prekážky).
        cv::Mat occupancy_;

    private:
        cv::Mat map_;
        double  resolution_;
    };

} // namespace environment