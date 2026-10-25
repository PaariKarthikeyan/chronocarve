#pragma once

#include "carver_base.hpp"

namespace chronocarve {

class ForwardDPCarver : public CarverBase {
public:
    ImageRetargetResult retarget(
        const cv::Mat& input_image,
        const RetargetConfig& config,
        std::function<void(double, const std::string&)> progress_callback = nullptr
    ) override;

    AlgorithmType get_type() const override { return AlgorithmType::FORWARD_DP; }
    std::string get_name() const override { return "Algorithm 3: Forward-Energy DP (Rubinstein et al. 2008)"; }
};

} // namespace chronocarve
