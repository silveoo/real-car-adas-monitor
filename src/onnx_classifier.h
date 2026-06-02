#pragma once

#include <onnxruntime_cxx_api.h>

#include <array>
#include <memory>
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
    Ort::Env env;
    Ort::SessionOptions sessionOptions;
    std::unique_ptr<Ort::Session> session;

    std::vector<float> mean;
    std::vector<float> stdValues;

    bool loaded;

    std::vector<float> extractArrayFromJson(const std::string& json,
                                            const std::string& key);

    std::vector<float> softmax(const std::vector<float>& logits);

    std::wstring toWideString(const std::string& value);
};