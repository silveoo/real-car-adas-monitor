#include "dms_hud.h"

void DMSHUD::draw(cv::Mat& frame, const DriverState& state) {
    cv::Scalar green(0, 255, 0);
    cv::Scalar red(0, 0, 255);
    cv::Scalar orange(0, 165, 255);
    cv::Scalar white(230, 230, 230);

    if (state.faceDetected) {
        cv::Scalar boxColor = state.alertDrowsy ? orange : green;
        drawCornerBox(frame, state.faceRect, boxColor);
    }

    int y = 30;

    cv::putText(frame,
                state.faceDetected ? "Face: detected" : "Face: not found",
                {20, y},
                cv::FONT_HERSHEY_SIMPLEX,
                0.7,
                state.faceDetected ? green : red,
                2);

    y += 35;

    cv::putText(frame,
                state.eyesOpen ? "Eyes: open" : "Eyes: closed",
                {20, y},
                cv::FONT_HERSHEY_SIMPLEX,
                0.7,
                state.eyesOpen ? green : orange,
                2);

    y += 35;

    cv::putText(frame,
                state.lookingForward ? "Head: forward" : "Head: turned",
                {20, y},
                cv::FONT_HERSHEY_SIMPLEX,
                0.7,
                state.lookingForward ? green : red,
                2);

    y += 35;

    cv::putText(frame,
                "Head angle: " + std::to_string(static_cast<int>(state.headTurnDeg)),
                {20, y},
                cv::FONT_HERSHEY_SIMPLEX,
                0.7,
                white,
                2);

    if (state.alertDrowsy) {
        cv::rectangle(frame, cv::Rect(0, 0, frame.cols, frame.rows), orange, 8);
        cv::putText(frame,
                    "DROWSINESS ALERT",
                    {frame.cols / 2 - 180, frame.rows / 2},
                    cv::FONT_HERSHEY_SIMPLEX,
                    1.1,
                    orange,
                    3);
    }

    if (state.alertDistracted) {
        cv::rectangle(frame, cv::Rect(0, frame.rows - 50, frame.cols, 50), red, cv::FILLED);
        cv::putText(frame,
                    "DISTRACTION ALERT",
                    {frame.cols / 2 - 170, frame.rows - 15},
                    cv::FONT_HERSHEY_SIMPLEX,
                    1.0,
                    cv::Scalar(255, 255, 255),
                    3);
    }
}

void DMSHUD::drawCornerBox(cv::Mat& frame, const cv::Rect& rect, const cv::Scalar& color) {
    int len = 30;
    int thickness = 3;

    cv::Point tl(rect.x, rect.y);
    cv::Point tr(rect.x + rect.width, rect.y);
    cv::Point bl(rect.x, rect.y + rect.height);
    cv::Point br(rect.x + rect.width, rect.y + rect.height);

    cv::line(frame, tl, {tl.x + len, tl.y}, color, thickness);
    cv::line(frame, tl, {tl.x, tl.y + len}, color, thickness);

    cv::line(frame, tr, {tr.x - len, tr.y}, color, thickness);
    cv::line(frame, tr, {tr.x, tr.y + len}, color, thickness);

    cv::line(frame, bl, {bl.x + len, bl.y}, color, thickness);
    cv::line(frame, bl, {bl.x, bl.y - len}, color, thickness);

    cv::line(frame, br, {br.x - len, br.y}, color, thickness);
    cv::line(frame, br, {br.x, br.y - len}, color, thickness);
}