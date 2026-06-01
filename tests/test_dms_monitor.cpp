#include "dms_monitor.h"

#include <gtest/gtest.h>

TEST(DMSMonitorTest, ModuleIsNotLoadedByDefault) {
    DMSMonitor monitor;

    EXPECT_FALSE(monitor.isLoaded());
}

TEST(DMSMonitorTest, AnalyzeEmptyFrameDoesNotCrash) {
    DMSMonitor monitor;

    cv::Mat emptyFrame;
    DriverState state = monitor.analyze(emptyFrame);

    EXPECT_FALSE(state.faceDetected);
}

TEST(DMSMonitorTest, LoadsModelsIfFilesExist) {
    DMSMonitor monitor;

    bool loaded = monitor.loadModels(
        "../models/haarcascade_frontalface_default.xml",
        "../models/haarcascade_eye.xml"
    );

    EXPECT_TRUE(loaded);
}