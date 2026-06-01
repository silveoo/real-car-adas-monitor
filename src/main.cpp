#include "dashboard.h"

#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
    std::cout << "Dashboard test started" << std::endl;

    cv::Mat frame(480, 640, CV_8UC3, cv::Scalar(20, 20, 20));

    OBDRecord record{};
    record.speedKmh = 87;
    record.engineRpm = 3200;
    record.throttlePos = 45;
    record.coolantTemp = 92;
    record.fuelLevel = 68;
    record.intakeAirTemp = 25;
    record.label = DrivingStyle::NORMAL;

    ClassificationResult classification{};
    classification.label = 1;
    classification.confidence = 0.91f;
    classification.scores = {0.03f, 0.91f, 0.06f};

    Dashboard dashboard;
    dashboard.draw(frame, record, classification);

    bool saved = cv::imwrite("../output/dashboard_test.png", frame);
    std::cout << "Image saved: " << saved << std::endl;

    return 0;
}