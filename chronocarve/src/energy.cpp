#include "energy.hpp"
#include <cmath>

namespace chronocarve {

cv::Mat EnergyComputer::to_grayscale(const cv::Mat& image) {
    if (image.channels() == 1) {
        return image;
    }
    cv::Mat gray;
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    return gray;
}

cv::Mat EnergyComputer::compute_backward_energy(const cv::Mat& image, double alpha) {
    cv::Mat gray = to_grayscale(image);

    cv::Mat grad_x, grad_y;
    cv::Sobel(gray, grad_x, CV_32F, 1, 0, 3);
    cv::Sobel(gray, grad_y, CV_32F, 0, 1, 3);

    cv::Mat energy = cv::abs(grad_x) + cv::abs(grad_y);
    if (alpha != 1.0) {
        energy *= alpha;
    }
    return energy;
}

void EnergyComputer::compute_forward_energy_costs(
    const cv::Mat& gray,
    cv::Mat& c_u,
    cv::Mat& c_l,
    cv::Mat& c_r
) {
    cv::Mat g;
    if (gray.type() != CV_32F) {
        gray.convertTo(g, CV_32F);
    } else {
        g = gray;
    }

    const int H = g.rows;
    const int W = g.cols;

    c_u.create(H, W, CV_32F);
    c_l.create(H, W, CV_32F);
    c_r.create(H, W, CV_32F);

    for (int i = 0; i < H; ++i) {
        const float* curr_row = g.ptr<float>(i);
        const float* prev_row = (i > 0) ? g.ptr<float>(i - 1) : curr_row;

        float* cu_ptr = c_u.ptr<float>(i);
        float* cl_ptr = c_l.ptr<float>(i);
        float* cr_ptr = c_r.ptr<float>(i);

        for (int j = 0; j < W; ++j) {
            float val_left  = (j > 0)     ? curr_row[j - 1] : curr_row[j];
            float val_right = (j < W - 1) ? curr_row[j + 1] : curr_row[j];
            float val_up    = prev_row[j];

            float cu = std::abs(val_right - val_left);
            float cl = cu + std::abs(val_up - val_left);
            float cr = cu + std::abs(val_up - val_right);

            cu_ptr[j] = cu;
            cl_ptr[j] = cl;
            cr_ptr[j] = cr;
        }
    }
}

cv::Mat EnergyComputer::generate_heatmap_vis(const cv::Mat& energy_map, int colormap) {
    cv::Mat norm_energy;
    double min_v, max_v;
    cv::minMaxLoc(energy_map, &min_v, &max_v);

    if (max_v > min_v) {
        energy_map.convertTo(norm_energy, CV_8U, 255.0 / (max_v - min_v), -min_v * 255.0 / (max_v - min_v));
    } else {
        norm_energy = cv::Mat::zeros(energy_map.size(), CV_8U);
    }

    cv::Mat heatmap;
    cv::applyColorMap(norm_energy, heatmap, colormap);
    return heatmap;
}

} // namespace chronocarve
