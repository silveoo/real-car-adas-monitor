#pragma once

#include "obd_parser.h"
#include "onnx_classifier.h"

#include <opencv2/opencv.hpp>
#include <string>

class Dashboard {
public:
    void draw(cv::Mat& frame,
              const OBDRecord& record,
              const ClassificationResult& classification);

private:
    void drawGauge(cv::Mat& frame,
                   cv::Point center,
                   int radius,
                   double value,
                   double maxValue,
                   const std::string& title,
                   const std::string& unit);

    void drawLinearGauge(cv::Mat& frame,
                         cv::Point origin,
                         int width,
                         int height,
                         double value,
                         double maxValue,
                         const std::string& title);

    cv::Scalar styleColor(int label);
    std::string styleText(int label);
};