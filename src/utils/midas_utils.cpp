#include "utils/midas_utils.hpp"
#include <iostream>


MiDaSDepth::MiDaSDepth(const std::string& model_path, bool use_cuda)
    : use_cuda_(use_cuda),
      device_((use_cuda && torch::cuda::is_available()) ? torch::kCUDA : torch::kCPU) {
        // device_(torch::kCUDA) {
    try {
        model_ = torch::jit::load(model_path);
        model_.to(device_);
        model_.eval();
        std::cout << "Loaded MiDaS model from " << model_path << std::endl;
    }
    catch (const c10::Error& e) {
        std::cerr << "Error loading the MiDaS model: " << e.what() << std::endl;
        throw;
    }
}

torch::Tensor MiDaSDepth::preprocess(const cv::Mat& frame) {
    // Convert BGR to RGB and resize to model input size (MiDaS_small is 256x256)
    cv::Mat rgb_img;
    cv::cvtColor(frame, rgb_img, cv::COLOR_BGR2RGB);
    cv::Mat resized_img;
    cv::resize(rgb_img, resized_img, cv::Size(256, 256), 0, 0, cv::INTER_CUBIC);

    // Convert to float, normalize to [0, 1], and convert to tensor
    torch::Tensor tensor_image = torch::from_blob(resized_img.data, {1, resized_img.rows, resized_img.cols, 3}, torch::kUInt8);
    tensor_image = tensor_image.permute({0, 3, 1, 2}).to(torch::kFloat32).div(255.0);
    return tensor_image.to(device_);
}

cv::Mat MiDaSDepth::postprocess(const torch::Tensor& prediction, const cv::Size& orig_size) {
    // Prediction shape: [1, 1, H, W], squeeze and interpolate to original frame size
    torch::Tensor pred_tensor = prediction;
    if (pred_tensor.dim() == 3) {
        pred_tensor = pred_tensor.unsqueeze(1); // [N, 1, H, W]
    } else if (pred_tensor.dim() == 2) {
        pred_tensor = pred_tensor.unsqueeze(0).unsqueeze(0); // [1, 1, H, W]
    }

    // Now interpolate
    torch::Tensor pred = torch::nn::functional::interpolate(
        pred_tensor,
        torch::nn::functional::InterpolateFuncOptions()
            .size(std::vector<int64_t>{orig_size.height, orig_size.width})
            .mode(torch::kBicubic)
            .align_corners(false)
    ).squeeze();


    // Convert inverse depth to relative depth and normalize to 0-255
    auto output = 1.0 / (pred + 1e-8);
    output = (output - output.min()) / (output.max() - output.min() + 1e-8);
    output = output.mul(255).clamp(0, 255).to(torch::kU8);

    // Convert to OpenCV Mat and return
    cv::Mat depth_map(orig_size.height, orig_size.width, CV_8UC1);
    std::memcpy((void*)depth_map.data, output.cpu().data_ptr(), sizeof(uint8_t) * orig_size.height * orig_size.width);
    return depth_map;
}

cv::Mat MiDaSDepth::getDepthMap(const cv::Mat& frame) {
    torch::NoGradGuard no_grad;
    torch::Tensor input_tensor = preprocess(frame);
    //std::cout << "preprocess done  "<<std::endl;

    torch::Tensor prediction ;
    prediction = model_.forward({input_tensor}).toTensor();
    // try {
    //     std::cout << "Before model forward" << std::endl << std::flush;
        
    //     cudaError_t err = cudaGetLastError();
    //     if (err != cudaSuccess) {
    //         std::cerr << "CUDA error: " << cudaGetErrorString(err) << std::endl << std::flush;
    //     }
    //     std::cout << "model forward done" << std::endl << std::flush;
    // } catch (const c10::Error &e) {
    //     std::cerr << "Model forward error: " << e.what() << std::endl << std::flush;
    //     return cv::Mat();
    // }

    cv::Mat depth_map = postprocess(prediction, frame.size());

    return depth_map;
}