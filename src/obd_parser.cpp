#include "obd_parser.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

DrivingStyle OBDParser::labelFromString(const std::string& label) {
    if (label == "SLOW") {
        return DrivingStyle::SLOW;
    }

    if (label == "NORMAL") {
        return DrivingStyle::NORMAL;
    }

    if (label == "AGGRESSIVE") {
        return DrivingStyle::AGGRESSIVE;
    }

    return DrivingStyle::UNKNOWN;
}

int OBDParser::load(const std::string& filePath) {
    records.clear();

    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return -1;
    }

    std::string line;

    // Skip CSV header
    std::getline(file, line);

    int lineNumber = 1;

    while (std::getline(file, line)) {
        lineNumber++;

        if (line.empty()) {
            continue;
        }

        std::stringstream ss(line);
        std::string token;
        std::vector<std::string> columns;

        while (std::getline(ss, token, ',')) {
            columns.push_back(token);
        }

        if (columns.size() != 7) {
            std::cerr << "Invalid column count at line " << lineNumber << ": " << line << std::endl;
            continue;
        }

        try {
            OBDRecord record{};
            record.speedKmh = std::stod(columns[0]);
            record.engineRpm = std::stod(columns[1]);
            record.throttlePos = std::stod(columns[2]);
            record.coolantTemp = std::stod(columns[3]);
            record.fuelLevel = std::stod(columns[4]);
            record.intakeAirTemp = std::stod(columns[5]);
            record.label = labelFromString(columns[6]);

            if (record.label == DrivingStyle::UNKNOWN) {
                std::cerr << "Unknown label at line " << lineNumber << ": " << columns[6] << std::endl;
                continue;
            }

            records.push_back(record);
        } catch (const std::exception& e) {
            std::cerr << "Invalid data at line " << lineNumber << ": " << e.what() << std::endl;
        }
    }

    return static_cast<int>(records.size());
}

const OBDRecord& OBDParser::getRecord(int index) const {
    if (index < 0 || index >= static_cast<int>(records.size())) {
        throw std::out_of_range("OBD record index out of range");
    }

    return records[index];
}

int OBDParser::size() const {
    return static_cast<int>(records.size());
}