#include "obd_parser.h"

#include <gtest/gtest.h>

#include <cstdio>
#include <fstream>
#include <string>

TEST(OBDParserTest, ConvertsLabelsCorrectly) {
    EXPECT_EQ(OBDParser::labelFromString("SLOW"), DrivingStyle::SLOW);
    EXPECT_EQ(OBDParser::labelFromString("NORMAL"), DrivingStyle::NORMAL);
    EXPECT_EQ(OBDParser::labelFromString("AGGRESSIVE"), DrivingStyle::AGGRESSIVE);
    EXPECT_EQ(OBDParser::labelFromString("UNKNOWN_VALUE"), DrivingStyle::UNKNOWN);
}

TEST(OBDParserTest, MissingFileReturnsMinusOne) {
    OBDParser parser;

    int result = parser.load("file_that_does_not_exist.csv");

    EXPECT_EQ(result, -1);
}

TEST(OBDParserTest, GetRecordThrowsWhenIndexIsInvalid) {
    OBDParser parser;

    EXPECT_THROW(parser.getRecord(0), std::out_of_range);
    EXPECT_THROW(parser.getRecord(-1), std::out_of_range);
}

TEST(OBDParserTest, ParsesValidCsvFile) {
    const std::string fileName = "valid_obd_test.csv";

    {
        std::ofstream file(fileName);
        file << "speed_kmh,engine_rpm,throttle_pos,coolant_temp,fuel_level,intake_air_temp,label\n";
        file << "87,3200,45,92,68,25,NORMAL\n";
        file << "40,1800,20,85,90,22,SLOW\n";
    }

    OBDParser parser;
    int count = parser.load(fileName);

    EXPECT_EQ(count, 2);
    EXPECT_EQ(parser.size(), 2);

    const OBDRecord& first = parser.getRecord(0);
    EXPECT_DOUBLE_EQ(first.speedKmh, 87);
    EXPECT_DOUBLE_EQ(first.engineRpm, 3200);
    EXPECT_DOUBLE_EQ(first.throttlePos, 45);
    EXPECT_DOUBLE_EQ(first.coolantTemp, 92);
    EXPECT_DOUBLE_EQ(first.fuelLevel, 68);
    EXPECT_DOUBLE_EQ(first.intakeAirTemp, 25);
    EXPECT_EQ(first.label, DrivingStyle::NORMAL);

    std::remove(fileName.c_str());
}

TEST(OBDParserTest, SkipsInvalidRows) {
    const std::string fileName = "invalid_rows_obd_test.csv";

    {
        std::ofstream file(fileName);
        file << "speed_kmh,engine_rpm,throttle_pos,coolant_temp,fuel_level,intake_air_temp,label\n";
        file << "87,3200,45,92,68,25,NORMAL\n";
        file << "bad,row,with,invalid,data\n";
        file << "120,5000,80,98,40,30,AGGRESSIVE\n";
        file << "50,2000,30,88,70,24,UNKNOWN_LABEL\n";
    }

    OBDParser parser;
    int count = parser.load(fileName);

    EXPECT_EQ(count, 2);
    EXPECT_EQ(parser.size(), 2);
    EXPECT_EQ(parser.getRecord(0).label, DrivingStyle::NORMAL);
    EXPECT_EQ(parser.getRecord(1).label, DrivingStyle::AGGRESSIVE);

    std::remove(fileName.c_str());
}