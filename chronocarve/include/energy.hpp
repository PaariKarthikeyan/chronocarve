#pragma once

#include <opencv2/opencv.hpp>
#include "common.hpp"

namespace chronocarve {

class EnergyComputer {
public:
    // Computes classic Avidan & Shamir gradient energy:
    // e1(x, y) = |dI/dx| + |dI/dy|
    // Returns CV_32F matrix of size H x W
    static cv::Mat compute_backward_energy(const cv::Mat& image, double alpha = 1.0);

    // Computes Rubinstein Forward Energy costs:
    // C_U(i, j) = |I(i, j+1) - I(i, j-1)|
    // C_L(i, j) = C_U(i, j) + |I(i-1, j) - I(i, j-1)|
    // C_R(i, j) = C_U(i, j) + |I(i-1, j) - I(i, j+1)|
    // Outputs matrices of type CV_32F
    static void compute_forward_energy_costs(
        const cv::Mat& gray,
        cv::Mat& c_u,
        cv::Mat& c_l,
        cv::Mat& c_r
    );

    // Converts an energy matrix (CV_32F) into a visually stunning Colormap Heatmap (CV_8UC3)
    static cv::Mat generate_heatmap_vis(const cv::Mat& energy_map, int colormap = cv::COLORMAP_TURBO);

    // Grayscale helper
    static cv::Mat to_grayscale(const cv::Mat& image);
};

} // namespace chronocarve
