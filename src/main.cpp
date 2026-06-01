#include "obd_parser.h"
#include "onnx_classifier.h"

#include <array>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>

std::string styleToString(int label) {
    switch (label) {
        case 0:
            return "SLOW";
        case 1:
            return "NORMAL";
        case 2:
            return "AGGRESSIVE";
        default:
            return "UNKNOWN";
    }
}

int drivingStyleToInt(DrivingStyle style) {
    return static_cast<int>(style);
}

int main() {
    std::cout << "Real Car ADAS Monitor v0.1.0" << std::endl;

    OBDParser parser;
    int loadedCount = parser.load("../data/obd_data.csv");

    if (loadedCount == -1) {
        std::cout << "CSV file was not found. Put obd_data.csv into data/ folder." << std::endl;
        return 0;
    }

    ONNXClassifier classifier;
    bool classifierLoaded = classifier.loadModel(
        "../models/driver_classifier.onnx",
        "../models/normalization_params.json"
    );

    if (!classifierLoaded) {
        std::cout << "Classifier files were not found. Put files into models/ folder." << std::endl;
        return 0;
    }

    std::cout << "Loaded records: " << loadedCount << std::endl;
    std::cout << "First 20 classifications:" << std::endl;

    int correct = 0;
    int limit = std::min(20, parser.size());

    std::cout
        << std::left
        << std::setw(5) << "#"
        << std::setw(14) << "True"
        << std::setw(14) << "Predicted"
        << std::setw(12) << "Confidence"
        << std::endl;

    for (int i = 0; i < limit; i++) {
        const OBDRecord& record = parser.getRecord(i);

        std::array<float, 6> features = {
            static_cast<float>(record.speedKmh),
            static_cast<float>(record.engineRpm),
            static_cast<float>(record.throttlePos),
            static_cast<float>(record.coolantTemp),
            static_cast<float>(record.fuelLevel),
            static_cast<float>(record.intakeAirTemp)
        };

        ClassificationResult result = classifier.classify(features);

        int trueLabel = drivingStyleToInt(record.label);
        if (result.label == trueLabel) {
            correct++;
        }

        std::cout
            << std::left
            << std::setw(5) << i + 1
            << std::setw(14) << styleToString(trueLabel)
            << std::setw(14) << styleToString(result.label)
            << std::setw(12) << std::fixed << std::setprecision(2) << result.confidence
            << std::endl;
    }

    double accuracy = limit == 0 ? 0.0 : static_cast<double>(correct) / limit * 100.0;

    std::cout << "Accuracy on first " << limit << " records: "
              << std::fixed << std::setprecision(2)
              << accuracy << "%" << std::endl;

    return 0;
}