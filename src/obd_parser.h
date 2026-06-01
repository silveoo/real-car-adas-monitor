#pragma once

#include <string>
#include <vector>

enum class DrivingStyle {
    SLOW = 0,
    NORMAL = 1,
    AGGRESSIVE = 2,
    UNKNOWN = -1
};

struct OBDRecord {
    double speedKmh;
    double engineRpm;
    double throttlePos;
    double coolantTemp;
    double fuelLevel;
    double intakeAirTemp;
    DrivingStyle label;
};

class OBDParser {
public:
    int load(const std::string& filePath);

    const OBDRecord& getRecord(int index) const;

    int size() const;

    static DrivingStyle labelFromString(const std::string& label);

private:
    std::vector<OBDRecord> records;
};