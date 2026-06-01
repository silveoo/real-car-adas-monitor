#include "dashboard.h"
#include "dms_hud.h"
#include "dms_monitor.h"
#include "obd_parser.h"
#include "onnx_classifier.h"

#include <opencv2/opencv.hpp>

#include <array>
#include <atomic>
#include <chrono>
#include <fstream>
#include <iostream>
#include <mutex>
#include <thread>

struct SharedState {
    OBDRecord currentRecord{};
    ClassificationResult currentClassification{};
    std::atomic<bool> running{true};
    std::mutex mutex;

    int processedRecords = 0;
    int aggressiveAlerts = 0;
};

void obdThreadFunction(SharedState& state,
                       OBDParser& parser,
                       ONNXClassifier& classifier) {
    int index = 0;

    while (state.running) {
        const OBDRecord& record = parser.getRecord(index);

        std::array<float, 6> features = {
            static_cast<float>(record.speedKmh),
            static_cast<float>(record.engineRpm),
            static_cast<float>(record.throttlePos),
            static_cast<float>(record.coolantTemp),
            static_cast<float>(record.fuelLevel),
            static_cast<float>(record.intakeAirTemp)
        };

        ClassificationResult classification = classifier.classify(features);

        static OBDRecord smoothRecord = record;

        smoothRecord.speedKmh = smoothRecord.speedKmh * 0.85 + record.speedKmh * 0.15;
        smoothRecord.engineRpm = smoothRecord.engineRpm * 0.85 + record.engineRpm * 0.15;
        smoothRecord.throttlePos = smoothRecord.throttlePos * 0.85 + record.throttlePos * 0.15;
        smoothRecord.coolantTemp = smoothRecord.coolantTemp * 0.95 + record.coolantTemp * 0.05;
        smoothRecord.fuelLevel = smoothRecord.fuelLevel * 0.98 + record.fuelLevel * 0.02;
        smoothRecord.intakeAirTemp = smoothRecord.intakeAirTemp * 0.95 + record.intakeAirTemp * 0.05;
        smoothRecord.label = record.label;

        {
            std::lock_guard<std::mutex> lock(state.mutex);
            state.currentRecord = record;
            state.currentClassification = classification;
            state.processedRecords++;

            if (classification.label == 2) {
                state.aggressiveAlerts++;
            }
        }

        index = (index + 1) % parser.size();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

int main() {
    std::cout << "Final ADAS system started" << std::endl;

    OBDParser parser;
    if (parser.load("../data/obd_data.csv") <= 0) {
        std::cout << "Failed to load CSV" << std::endl;
        return 1;
    }

    ONNXClassifier classifier;
    if (!classifier.loadModel(
            "../models/driver_classifier.onnx",
            "../models/normalization_params.json")) {
        std::cout << "Failed to load classifier" << std::endl;
        return 1;
    }

    DMSMonitor dmsMonitor;
    if (!dmsMonitor.loadModels(
            "../models/haarcascade_frontalface_default.xml",
            "../models/haarcascade_eye.xml")) {
        std::cout << "Failed to load DMS models" << std::endl;
        return 1;
    }

    cv::VideoCapture camera(0, cv::CAP_DSHOW);
    if (!camera.isOpened()) {
        std::cout << "Failed to open camera" << std::endl;
        return 1;
    }

    cv::VideoWriter writer(
        "../output/result_situation2.avi",
        cv::VideoWriter::fourcc('M', 'J', 'P', 'G'),
        20.0,
        cv::Size(1280, 480)
    );

    if (!writer.isOpened()) {
        std::cout << "Failed to open video writer" << std::endl;
        return 1;
    }

    std::ofstream alertLog("../output/dms_alerts.log");

    SharedState sharedState;

    {
        std::lock_guard<std::mutex> lock(sharedState.mutex);
        sharedState.currentRecord = parser.getRecord(0);
        sharedState.currentClassification = classifier.classify({
            static_cast<float>(sharedState.currentRecord.speedKmh),
            static_cast<float>(sharedState.currentRecord.engineRpm),
            static_cast<float>(sharedState.currentRecord.throttlePos),
            static_cast<float>(sharedState.currentRecord.coolantTemp),
            static_cast<float>(sharedState.currentRecord.fuelLevel),
            static_cast<float>(sharedState.currentRecord.intakeAirTemp)
        });
    }

    std::thread obdThread(
        obdThreadFunction,
        std::ref(sharedState),
        std::ref(parser),
        std::ref(classifier)
    );

    Dashboard dashboard;
    DMSHUD dmsHud;

    auto startTime = std::chrono::steady_clock::now();

    int totalAlerts = 0;
    int drowsyAlerts = 0;
    int distractedAlerts = 0;

    std::cout << "Recording final video..." << std::endl;

    for (int frameIndex = 0; frameIndex < 300; frameIndex++) {
        cv::Mat cameraFrame;
        camera >> cameraFrame;

        if (cameraFrame.empty()) {
            break;
        }

        cv::resize(cameraFrame, cameraFrame, cv::Size(640, 480));

        DriverState driverState = dmsMonitor.analyze(cameraFrame);
        dmsHud.draw(cameraFrame, driverState);

        OBDRecord recordCopy{};
        ClassificationResult classificationCopy{};

        {
            std::lock_guard<std::mutex> lock(sharedState.mutex);
            recordCopy = sharedState.currentRecord;
            classificationCopy = sharedState.currentClassification;
        }

        cv::Mat dashboardFrame(480, 640, CV_8UC3, cv::Scalar(20, 20, 20));
        dashboard.draw(dashboardFrame, recordCopy, classificationCopy);

        cv::Mat finalFrame(480, 1280, CV_8UC3);
        dashboardFrame.copyTo(finalFrame(cv::Rect(0, 0, 640, 480)));
        cameraFrame.copyTo(finalFrame(cv::Rect(640, 0, 640, 480)));

        if (driverState.alertDrowsy || driverState.alertDistracted || classificationCopy.label == 2) {
            totalAlerts++;

            if (driverState.alertDrowsy) {
                drowsyAlerts++;
                alertLog << "Frame " << frameIndex << ": DROWSINESS ALERT\n";
            }

            if (driverState.alertDistracted) {
                distractedAlerts++;
                alertLog << "Frame " << frameIndex << ": DISTRACTION ALERT\n";
            }

            if (classificationCopy.label == 2) {
                alertLog << "Frame " << frameIndex << ": AGGRESSIVE DRIVING\n";
            }
        }

        if (frameIndex == 30) {
            cv::imwrite("../output/final_normal.png", finalFrame);
        }

        if (frameIndex == 120) {
            cv::imwrite("../output/final_situation.png", finalFrame);
        }

        writer.write(finalFrame);
    }

    sharedState.running = false;
    obdThread.join();

    auto endTime = std::chrono::steady_clock::now();
    double seconds = std::chrono::duration<double>(endTime - startTime).count();

    std::cout << "Final video saved: ../output/result_situation2.avi" << std::endl;
    std::cout << "Alert log saved: ../output/dms_alerts.log" << std::endl;
    std::cout << "Runtime: " << seconds << " sec" << std::endl;
    std::cout << "Processed OBD records: " << sharedState.processedRecords << std::endl;
    std::cout << "Total alerts: " << totalAlerts << std::endl;
    std::cout << "Drowsy alerts: " << drowsyAlerts << std::endl;
    std::cout << "Distracted alerts: " << distractedAlerts << std::endl;
    std::cout << "Aggressive driving alerts: " << sharedState.aggressiveAlerts << std::endl;

    return 0;
}