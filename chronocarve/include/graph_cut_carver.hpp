#pragma once

#include "carver_base.hpp"

namespace chronocarve {

class GraphCutCarver : public CarverBase {
public:
    ImageRetargetResult retarget(
        const cv::Mat& input_image,
        const RetargetConfig& config,
        std::function<void(double, const std::string&)> progress_callback = nullptr
    ) override;

    AlgorithmType get_type() const override { return AlgorithmType::GRAPH_CUT_2D; }
    std::string get_name() const override { return "Algorithm 4: 2D Graph Cut Seam Carving (Boykov-Kolmogorov)"; }
};

} // namespace chronocarve
