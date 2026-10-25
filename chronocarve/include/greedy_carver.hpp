#pragma once

#include "carver_base.hpp"

namespace chronocarve {

class GreedyCarver : public CarverBase {
public:
    ImageRetargetResult retarget(
        const cv::Mat& input_image,
        const RetargetConfig& config,
        std::function<void(double, const std::string&)> progress_callback = nullptr
    ) override;

    AlgorithmType get_type() const override { return AlgorithmType::GREEDY; }
    std::string get_name() const override { return "Algorithm 1: Greedy Baseline (O(H))"; }
};

} // namespace chronocarve
