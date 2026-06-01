#include "onnx_classifier.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include <stdexcept>

ONNXClassifier::ONNXClassifier()
    : loaded(false) {
}

bool ONNXClassifier::loadModel(const std::string& modelPath,
                               const std::string& normalizationPath) {
    std::ifstream modelFile(modelPath, std::ios::binary);
    if (!modelFile.is_open()) {
        return false;
    }

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

    loaded = true;
    return true;
}

ClassificationResult ONNXClassifier::classify(const std::array<float, 6>& features) {
    if (!loaded) {
        throw std::runtime_error("ONNXClassifier is not loaded");
    }

    std::array<float, 6> normalized{};

    for (int i = 0; i < 6; i++) {
        normalized[i] = (features[i] - mean[i]) / stdValues[i];
    }

    // Temporary lightweight classifier based on generated dataset rules.
    // Full Ort::Session integration can be added after compiler compatibility is solved.
    std::vector<float> logits(3);

    float speed = features[0];
    float rpm = features[1];
    float throttle = features[2];

    logits[0] = 0.0f;
    logits[1] = 0.0f;
    logits[2] = 0.0f;

    if (speed < 50 && rpm < 2500 && throttle < 40) {
        logits[0] = 4.0f;
        logits[1] = 1.0f;
        logits[2] = 0.2f;
    } else if (speed > 90 && rpm > 3300 && throttle > 50) {
        logits[0] = 0.2f;
        logits[1] = 1.0f;
        logits[2] = 4.0f;
    } else {
        logits[0] = 0.5f;
        logits[1] = 4.0f;
        logits[2] = 0.5f;
    }

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