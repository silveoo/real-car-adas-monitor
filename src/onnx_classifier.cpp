#include "onnx_classifier.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>

ONNXClassifier::ONNXClassifier()
    : env(ORT_LOGGING_LEVEL_WARNING, "RealCarMonitor"),
      loaded(false) {
    sessionOptions.SetIntraOpNumThreads(1);
    sessionOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_EXTENDED);
}

bool ONNXClassifier::loadModel(const std::string& modelPath,
                               const std::string& normalizationPath) {
    std::ifstream jsonFile(normalizationPath);
    if (!jsonFile.is_open()) {
        return false;
    }

    std::stringstream buffer;
    buffer << jsonFile.rdbuf();
    std::string json = buffer.str();

    mean = extractArrayFromJson(json, "mean");
    stdValues = extractArrayFromJson(json, "std");

    if (mean.size() != 6 || stdValues.size() != 6) {
        return false;
    }

    try {
        std::wstring wideModelPath = toWideString(modelPath);
        session = std::make_unique<Ort::Session>(
            env,
            wideModelPath.c_str(),
            sessionOptions
        );
    } catch (const Ort::Exception& e) {
        std::cerr << "ONNX Runtime error: " << e.what() << std::endl;
        loaded = false;
        return false;
    }

    loaded = true;
    return true;
}

ClassificationResult ONNXClassifier::classify(const std::array<float, 6>& features) {
    if (!loaded || !session) {
        throw std::runtime_error("ONNXClassifier is not loaded");
    }

    std::array<float, 6> normalized{};

    for (int i = 0; i < 6; i++) {
        normalized[i] = (features[i] - mean[i]) / stdValues[i];
    }

    std::array<int64_t, 2> inputShape = {1, 6};

    Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(
        OrtArenaAllocator,
        OrtMemTypeDefault
    );

    Ort::Value inputTensor = Ort::Value::CreateTensor<float>(
        memoryInfo,
        normalized.data(),
        normalized.size(),
        inputShape.data(),
        inputShape.size()
    );

    const char* inputNames[] = {"features"};
    const char* outputNames[] = {"class_scores"};

    auto outputTensors = session->Run(
        Ort::RunOptions{nullptr},
        inputNames,
        &inputTensor,
        1,
        outputNames,
        1
    );

    float* outputData = outputTensors.front().GetTensorMutableData<float>();

    std::vector<float> logits = {
        outputData[0],
        outputData[1],
        outputData[2]
    };

    std::vector<float> probabilities = softmax(logits);

    int label = static_cast<int>(
        std::distance(
            probabilities.begin(),
            std::max_element(probabilities.begin(), probabilities.end())
        )
    );

    ClassificationResult result{};
    result.label = label;
    result.confidence = probabilities[label];
    result.scores = probabilities;

    return result;
}

std::vector<float> ONNXClassifier::extractArrayFromJson(const std::string& json,
                                                        const std::string& key) {
    std::string quotedKey = "\"" + key + "\"";
    size_t keyPosition = json.find(quotedKey);

    if (keyPosition == std::string::npos) {
        return {};
    }

    size_t arrayStart = json.find('[', keyPosition);
    size_t arrayEnd = json.find(']', arrayStart);

    if (arrayStart == std::string::npos || arrayEnd == std::string::npos) {
        return {};
    }

    std::string arrayContent = json.substr(arrayStart + 1, arrayEnd - arrayStart - 1);

    std::vector<float> values;
    std::stringstream ss(arrayContent);
    std::string token;

    while (std::getline(ss, token, ',')) {
        values.push_back(std::stof(token));
    }

    return values;
}

std::vector<float> ONNXClassifier::softmax(const std::vector<float>& logits) {
    std::vector<float> result(logits.size());

    float maxLogit = *std::max_element(logits.begin(), logits.end());

    float sum = 0.0f;

    for (size_t i = 0; i < logits.size(); i++) {
        result[i] = std::exp(logits[i] - maxLogit);
        sum += result[i];
    }

    for (float& value : result) {
        value /= sum;
    }

    return result;
}

std::wstring ONNXClassifier::toWideString(const std::string& value) {
    return std::wstring(value.begin(), value.end());
}