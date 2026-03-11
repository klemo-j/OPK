//
// Created by kllemo on 2/25/26.
//
#include <opencv2/opencv.hpp>

#ifndef OPENCV_SHAPE_HPP
#define OPENCV_SHAPE_HPP


class Shape {
public:
    virtual void draw(cv::Mat& image) = 0;
};

class Circle : public Shape {
public:
    void draw(cv::Mat& image) override {
        cv::circle(image, center, radius, color, thickness);
    }
};
class Rectangle : public Shape {
public:
    void draw(cv::Mat& image) override {
        cv::rectangle(image, topLeft, bottomRight, color, thickness);
    }
};

#endif //OPENCV_SHAPE_HPP