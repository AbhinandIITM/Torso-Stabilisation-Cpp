#include "utils/midas_utils.hpp"
#include <opencv2/opencv.hpp>
#include <iostream>
#include "tools/cpp/runfiles/runfiles.h"

using bazel::tools::cpp::runfiles::Runfiles;

int main(int argc, char** argv) {
    std::string error;
    std::unique_ptr<Runfiles> runfiles(Runfiles::Create(argv[0], &error));
    if (!runfiles) {
        std::cerr << "Runfiles error: " << error << std::endl;
        return 1;
    }

    // WORKSPACE name is "mediapipe", so runfiles logical path starts with that
    const std::string model_path =
        runfiles->Rlocation("mediapipe/src/models/dpt_swin2_tiny_256_torchscript.pt");

    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "[ERROR] Failed to open webcam" << std::endl;
        return 1;
    }
    cap.set(cv::CAP_PROP_BUFFERSIZE, 1);

    MiDaSDepth midas(model_path, /*use_cuda=*/true);
    std::cout << "[INFO] MiDaS initialized with model: " << model_path << std::endl;
    std::cout << "[INFO] Press ESC to exit" << std::endl;

    cv::Mat frame, depth;
    while (true) {
        cap >> frame;
        if (frame.empty()) {
            std::cerr << "[WARN] Empty frame received from webcam" << std::endl;
            break;
        }

        depth = midas.getDepthMap(frame);
        if (depth.empty()) {
            std::cerr << "[WARN] Depth inference failed for current frame" << std::endl;
            continue;
        }

        cv::Mat depth_color;
        cv::applyColorMap(depth, depth_color, cv::COLORMAP_INFERNO);
        cv::imshow("Webcam", frame);
        cv::imshow("MiDaS Depth", depth_color);

        if (cv::waitKey(1) == 27) break;  // ESC
    }

    cap.release();
    cv::destroyAllWindows();
    return 0;
}
