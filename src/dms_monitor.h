#pragma once

#include <opencv2/opencv.hpp>
#include <deque>

struct DriverState {
    bool faceDetected = false;
    bool eyesOpen = true;
    bool lookingForward = true;

    float eyeOpenness = 1.0f;
    float headTurnDeg = 0.0f;

    bool alertDrowsy = false;
    bool alertDistracted = false;

    cv::Rect faceRect;
};

class DMSMonitor {
public:
    bool loadModels(const std::string& faceModelPath,
                    const std::string& eyeCascadePath);

    DriverState analyze(const cv::Mat& frame);

    bool isLoaded() const;

private:
    cv::CascadeClassifier eyeCascade;
    cv::CascadeClassifier faceCascade;

    std::deque<bool> eyeHistory;

    bool loaded = false;

    cv::Rect detectFace(const cv::Mat& gray);
    float estimateEyeOpenness(const cv::Mat& gray, const cv::Rect& faceRect);
    float estimateHeadTurn(const cv::Mat& frame, const cv::Rect& faceRect);
};