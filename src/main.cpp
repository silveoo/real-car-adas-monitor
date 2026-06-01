#include "obd_parser.h"

#include <iostream>
#include <map>
#include <string>

std::string styleToString(DrivingStyle style) {
    switch (style) {
        case DrivingStyle::SLOW:
            return "SLOW";
        case DrivingStyle::NORMAL:
            return "NORMAL";
        case DrivingStyle::AGGRESSIVE:
            return "AGGRESSIVE";
        default:
            return "UNKNOWN";
    }
}

int main() {
    std::cout << "Real Car ADAS Monitor v0.1.0" << std::endl;

    OBDParser parser;
    int loadedCount = parser.load("../data/obd_data.csv");

    if (loadedCount == -1) {
        std::cout << "CSV file was not found. Put obd_data.csv into data/ folder." << std::endl;
        return 0;
    }

    std::cout << "Loaded records: " << loadedCount << std::endl;

    int limit = std::min(5, parser.size());

    for (int i = 0; i < limit; i++) {
        const OBDRecord& record = parser.getRecord(i);

        std::cout
            << "Record #" << i + 1
            << " speed=" << record.speedKmh
            << " rpm=" << record.engineRpm
            << " throttle=" << record.throttlePos
            << " temp=" << record.coolantTemp
            << " fuel=" << record.fuelLevel
            << " intakeTemp=" << record.intakeAirTemp
            << " label=" << styleToString(record.label)
            << std::endl;
    }

    std::map<DrivingStyle, int> classStats;

    for (int i = 0; i < parser.size(); i++) {
        classStats[parser.getRecord(i).label]++;
    }

    std::cout << "Class statistics:" << std::endl;
    std::cout << "SLOW: " << classStats[DrivingStyle::SLOW] << std::endl;
    std::cout << "NORMAL: " << classStats[DrivingStyle::NORMAL] << std::endl;
    std::cout << "AGGRESSIVE: " << classStats[DrivingStyle::AGGRESSIVE] << std::endl;

    return 0;
}