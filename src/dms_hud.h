#pragma once

#include "dms_monitor.h"

#include <opencv2/opencv.hpp>

class DMSHUD {
public:
    void draw(cv::Mat& frame, const DriverState& state);

private:
    void drawCornerBox(cv::Mat& frame, const cv::Rect& rect, const cv::Scalar& color);
};