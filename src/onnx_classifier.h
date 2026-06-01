#pragma once

#include <array>
#include <string>
#include <vector>

struct ClassificationResult {
    int label;
    float confidence;
    std::vector<float> scores;
};

class ONNXClassifier {
public:
    ONNXClassifier();

    bool loadModel(const std::string& modelPath,
                   const std::string& normalizationPath);

    ClassificationResult classify(const std::array<float, 6>& features);

private:
    std::vector<float> mean;
    std::vector<float> stdValues;

    bool loaded;

    std::vector<float> extractArrayFromJson(const std::string& json,
                                            const std::string& key);

    std::vector<float> softmax(const std::vector<float>& logits);
};