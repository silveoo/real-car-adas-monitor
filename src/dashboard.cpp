#include "dashboard.h"

#include <algorithm>
#include <cmath>

void Dashboard::draw(cv::Mat& frame,
                     const OBDRecord& record,
                     const ClassificationResult& classification) {
    cv::rectangle(frame, cv::Rect(0, 0, 640, 480), cv::Scalar(25, 25, 25), cv::FILLED);

    drawGauge(frame, {160, 150}, 90, record.speedKmh, 140, "SPEED", "km/h");
    drawGauge(frame, {470, 150}, 90, record.engineRpm, 6000, "RPM", "");

    drawLinearGauge(frame, {80, 285}, 480, 24, record.coolantTemp, 120, "TEMP");
    drawLinearGauge(frame, {80, 335}, 480, 24, record.fuelLevel, 100, "FUEL");
    drawLinearGauge(frame, {80, 385}, 480, 24, record.throttlePos, 100, "THROTTLE");

    cv::Scalar color = styleColor(classification.label);

    cv::putText(frame,
                "STYLE: " + styleText(classification.label),
                {80, 450},
                cv::FONT_HERSHEY_SIMPLEX,
                0.8,
                color,
                2);

    int percent = static_cast<int>(classification.confidence * 100.0f);
    cv::putText(frame,
                std::to_string(percent) + "%",
                {430, 450},
                cv::FONT_HERSHEY_SIMPLEX,
                0.8,
                color,
                2);

    if (record.coolantTemp > 100) {
        cv::putText(frame, "WARNING: HIGH TEMP", {80, 35},
                    cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 80, 255), 2);
    }

    if (record.fuelLevel < 15) {
        cv::putText(frame, "WARNING: LOW FUEL", {330, 35},
                    cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 80, 255), 2);
    }
}

void Dashboard::drawGauge(cv::Mat& frame,
                          cv::Point center,
                          int radius,
                          double value,
                          double maxValue,
                          const std::string& title,
                          const std::string& unit) {
    cv::circle(frame, center, radius, cv::Scalar(80, 80, 80), 2);

    double clamped = std::clamp(value, 0.0, maxValue);
    double angle = 225.0 + (clamped / maxValue) * 270.0;
    double rad = angle * CV_PI / 180.0;

    cv::Point needleEnd(
        center.x + static_cast<int>(std::cos(rad) * (radius - 15)),
        center.y + static_cast<int>(std::sin(rad) * (radius - 15))
    );

    cv::line(frame, center, needleEnd, cv::Scalar(0, 255, 0), 3);
    cv::circle(frame, center, 5, cv::Scalar(255, 255, 255), cv::FILLED);

    cv::putText(frame, title, {center.x - 45, center.y - 105},
                cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(220, 220, 220), 2);

    cv::putText(frame, std::to_string(static_cast<int>(value)) + " " + unit,
                {center.x - 55, center.y + 120},
                cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(220, 220, 220), 2);
}

void Dashboard::drawLinearGauge(cv::Mat& frame,
                                cv::Point origin,
                                int width,
                                int height,
                                double value,
                                double maxValue,
                                const std::string& title) {
    cv::putText(frame, title, {origin.x, origin.y - 8},
                cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(220, 220, 220), 1);

    cv::rectangle(frame, cv::Rect(origin.x, origin.y, width, height),
                  cv::Scalar(70, 70, 70), cv::FILLED);

    double clamped = std::clamp(value, 0.0, maxValue);
    int filled = static_cast<int>((clamped / maxValue) * width);

    cv::rectangle(frame, cv::Rect(origin.x, origin.y, filled, height),
                  cv::Scalar(0, 180, 255), cv::FILLED);

    cv::rectangle(frame, cv::Rect(origin.x, origin.y, width, height),
                  cv::Scalar(180, 180, 180), 1);

    cv::putText(frame, std::to_string(static_cast<int>(value)),
                {origin.x + width + 10, origin.y + height - 5},
                cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(220, 220, 220), 1);
}

cv::Scalar Dashboard::styleColor(int label) {
    if (label == 0) return cv::Scalar(255, 200, 0);
    if (label == 2) return cv::Scalar(0, 0, 255);
    return cv::Scalar(0, 255, 0);
}

std::string Dashboard::styleText(int label) {
    if (label == 0) return "SLOW";
    if (label == 2) return "AGGRESSIVE";
    return "NORMAL";
}