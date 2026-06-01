#include "dms_monitor.h"

#include <algorithm>

bool DMSMonitor::loadModels(const std::string& faceModelPath,
                            const std::string& eyeCascadePath) {
    bool faceLoaded = faceCascade.load(faceModelPath);
    bool eyesLoaded = eyeCascade.load(eyeCascadePath);

    loaded = faceLoaded && eyesLoaded;
    return loaded;
}

bool DMSMonitor::isLoaded() const {
    return loaded;
}

DriverState DMSMonitor::analyze(const cv::Mat& frame) {
    DriverState state{};

    if (frame.empty() || !loaded) {
        state.faceDetected = false;
        return state;
    }

    cv::Mat gray;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    cv::equalizeHist(gray, gray);

    cv::Rect face = detectFace(gray);

    if (face.area() <= 0) {
        state.faceDetected = false;
        return state;
    }

    state.faceDetected = true;
    state.faceRect = face;

    state.eyeOpenness = estimateEyeOpenness(gray, face);
    state.eyesOpen = state.eyeOpenness > 0.35f;

    state.headTurnDeg = estimateHeadTurn(frame, face);
    state.lookingForward = std::abs(state.headTurnDeg) < 18.0f;

    eyeHistory.push_back(state.eyesOpen);
    if (eyeHistory.size() > 15) {
        eyeHistory.pop_front();
    }

    int closedCount = 0;
    for (bool open : eyeHistory) {
        if (!open) {
            closedCount++;
        }
    }

    state.alertDrowsy = eyeHistory.size() >= 15 && closedCount >= 10;
    state.alertDistracted = !state.lookingForward;

    return state;
}

cv::Rect DMSMonitor::detectFace(const cv::Mat& gray) {
    std::vector<cv::Rect> faces;

    faceCascade.detectMultiScale(
        gray,
        faces,
        1.1,
        4,
        0,
        cv::Size(80, 80)
    );

    if (faces.empty()) {
        return {};
    }

    return *std::max_element(
        faces.begin(),
        faces.end(),
        [](const cv::Rect& a, const cv::Rect& b) {
            return a.area() < b.area();
        }
    );
}

float DMSMonitor::estimateEyeOpenness(const cv::Mat& gray, const cv::Rect& faceRect) {
    cv::Rect upperFace(
        faceRect.x,
        faceRect.y,
        faceRect.width,
        faceRect.height / 2
    );

    cv::Mat roi = gray(upperFace);

    std::vector<cv::Rect> eyes;
    eyeCascade.detectMultiScale(
        roi,
        eyes,
        1.1,
        3,
        0,
        cv::Size(15, 15)
    );

    if (eyes.size() >= 2) {
        return 1.0f;
    }

    if (eyes.size() == 1) {
        return 0.5f;
    }

    return 0.0f;
}

float DMSMonitor::estimateHeadTurn(const cv::Mat& frame, const cv::Rect& faceRect) {
    float frameCenterX = frame.cols / 2.0f;
    float faceCenterX = faceRect.x + faceRect.width / 2.0f;

    float normalizedOffset = (faceCenterX - frameCenterX) / frameCenterX;

    return normalizedOffset * 35.0f;
}