#include "dashboard.h"
#include "dms_hud.h"
#include "dms_monitor.h"
#include "obd_parser.h"
#include "onnx_classifier.h"

#include <opencv2/opencv.hpp>

#include <array>
#include <iostream>
#include <string>

int main() {
    std::cout << "Realtime ADAS started" << std::endl;

    OBDParser parser;

    if (parser.load("../data/obd_data.csv") <= 0) {
        std::cout << "CSV load failed" << std::endl;
        return 1;
    }

    ONNXClassifier classifier;

    if (!classifier.loadModel(
            "../models/driver_classifier.onnx",
            "../models/normalization_params.json")) {
        std::cout << "Classifier load failed" << std::endl;
        return 1;
    }

    DMSMonitor dms;

    if (!dms.loadModels(
            "../models/haarcascade_frontalface_default.xml",
            "../models/haarcascade_eye.xml")) {
        std::cout << "DMS models load failed" << std::endl;
        return 1;
    }

    cv::VideoCapture camera(0, cv::CAP_DSHOW);

    std::cout << "Camera opened: "
              << camera.isOpened()
              << std::endl;

    if (!camera.isOpened()) {
        return 1;
    }

    Dashboard dashboard;
    DMSHUD hud;

    cv::namedWindow(
        "Real Car ADAS Monitor",
        cv::WINDOW_NORMAL
    );

    cv::resizeWindow(
        "Real Car ADAS Monitor",
        1280,
        480
    );

    bool showDashboard = true;
    bool showDms = true;
    bool recording = true;

    int screenshotIndex = 0;
    int index = 0;

    cv::VideoWriter writer(
        "../output/realtime_recording.avi",
        cv::VideoWriter::fourcc('M', 'J', 'P', 'G'),
        20.0,
        cv::Size(1280, 480)
    );

    std::cout << "\nControls:\n";
    std::cout << "Q - Exit\n";
    std::cout << "S - Screenshot\n";
    std::cout << "R - Toggle recording\n";
    std::cout << "D - Toggle dashboard\n";
    std::cout << "M - Toggle DMS\n\n";

    while (true) {
        cv::Mat camFrame;
        camera >> camFrame;

        if (camFrame.empty()) {
            std::cout << "Empty camera frame" << std::endl;
            break;
        }

        cv::resize(
            camFrame,
            camFrame,
            cv::Size(640, 480)
        );

        const OBDRecord& record =
            parser.getRecord(index);

        index = (index + 1) % parser.size();

        std::array<float, 6> features = {
            static_cast<float>(record.speedKmh),
            static_cast<float>(record.engineRpm),
            static_cast<float>(record.throttlePos),
            static_cast<float>(record.coolantTemp),
            static_cast<float>(record.fuelLevel),
            static_cast<float>(record.intakeAirTemp)
        };

        ClassificationResult classification =
            classifier.classify(features);

        DriverState state{};

        if (showDms) {
            state = dms.analyze(camFrame);
            hud.draw(camFrame, state);
        }
        else {
            cv::putText(
                camFrame,
                "DMS DISABLED",
                {180, 240},
                cv::FONT_HERSHEY_SIMPLEX,
                1.0,
                cv::Scalar(255, 255, 255),
                2
            );
        }

        cv::Mat dashFrame(
            480,
            640,
            CV_8UC3,
            cv::Scalar(20, 20, 20)
        );

        if (showDashboard) {
            dashboard.draw(
                dashFrame,
                record,
                classification
            );
        }
        else {
            dashFrame.setTo(
                cv::Scalar(40, 40, 40)
            );

            cv::putText(
                dashFrame,
                "DASHBOARD DISABLED",
                {120, 240},
                cv::FONT_HERSHEY_SIMPLEX,
                1.0,
                cv::Scalar(255,255,255),
                2
            );
        }

        cv::Mat finalFrame(
            480,
            1280,
            CV_8UC3
        );

        dashFrame.copyTo(
            finalFrame(
                cv::Rect(
                    0,
                    0,
                    640,
                    480
                )
            )
        );

        camFrame.copyTo(
            finalFrame(
                cv::Rect(
                    640,
                    0,
                    640,
                    480
                )
            )
        );

        cv::putText(
            finalFrame,
            "[Q] Exit  [S] Screenshot  [R] Record  [D] Dashboard  [M] DMS",
            {10, 25},
            cv::FONT_HERSHEY_SIMPLEX,
            0.55,
            cv::Scalar(255,255,255),
            1
        );

        cv::imshow(
            "Real Car ADAS Monitor",
            finalFrame
        );

        if (recording && writer.isOpened()) {
            writer.write(finalFrame);
        }

        int key = cv::waitKey(30);

        if (key == 'q' ||
            key == 'Q' ||
            key == 27) {
            break;
        }

        if (key == 'd' ||
            key == 'D') {
            showDashboard = !showDashboard;

            std::cout
                << "Dashboard: "
                << (showDashboard ? "ON" : "OFF")
                << std::endl;
        }

        if (key == 'm' ||
            key == 'M') {
            showDms = !showDms;

            std::cout
                << "DMS: "
                << (showDms ? "ON" : "OFF")
                << std::endl;
        }

        if (key == 'r' ||
            key == 'R') {
            recording = !recording;

            std::cout
                << "Recording: "
                << (recording ? "ON" : "OFF")
                << std::endl;
        }

        if (key == 's' ||
            key == 'S') {
            std::string filename =
                "../output/screenshot_" +
                std::to_string(
                    screenshotIndex++
                ) +
                ".png";

            cv::imwrite(
                filename,
                finalFrame
            );

            std::cout
                << "Saved: "
                << filename
                << std::endl;
        }
    }

    writer.release();
    camera.release();
    cv::destroyAllWindows();

    return 0;
}