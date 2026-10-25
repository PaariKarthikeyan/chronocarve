#pragma once

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <functional>
#include <memory>
#include "common.hpp"

namespace chronocarve {

struct ImageRetargetResult {
    cv::Mat retargeted_image;
    cv::Mat seam_map_overlay;
    cv::Mat energy_heatmap;
    std::vector<SeamPath> seams_removed;
    BenchmarkMetrics metrics;
};

class CarverBase {
public:
    virtual ~CarverBase() = default;

    virtual ImageRetargetResult retarget(
        const cv::Mat& input_image,
        const RetargetConfig& config,
        std::function<void(double, const std::string&)> progress_callback = nullptr
    ) = 0;

    virtual AlgorithmType get_type() const = 0;
    virtual std::string get_name() const = 0;

    // Removes one vertical seam from a 1-channel or 3-channel frame
    static cv::Mat remove_seam(const cv::Mat& frame, const std::vector<int>& seam) {
        CV_Assert(!frame.empty());
        CV_Assert(frame.rows == static_cast<int>(seam.size()));

        const int H = frame.rows;
        const int W = frame.cols;
        CV_Assert(W > 1);

        cv::Mat output(H, W - 1, frame.type());
        const size_t elem_size = frame.elemSize();

        for (int i = 0; i < H; ++i) {
            int col = seam[i];
            if (col < 0) col = 0;
            if (col >= W) col = W - 1;

            const uchar* src_ptr = frame.ptr<uchar>(i);
            uchar* dst_ptr = output.ptr<uchar>(i);

            if (col > 0) {
                std::memcpy(dst_ptr, src_ptr, col * elem_size);
            }
            if (col < W - 1) {
                std::memcpy(dst_ptr + col * elem_size,
                            src_ptr + (col + 1) * elem_size,
                            (W - 1 - col) * elem_size);
            }
        }
        return output;
    }

    // Draws seam onto an image with customizable color
    static cv::Mat draw_seam(
        const cv::Mat& frame,
        const std::vector<int>& seam,
        const cv::Scalar& color = cv::Scalar(0, 240, 255)
    ) {
        cv::Mat vis = frame.clone();
        if (vis.channels() == 1) {
            cv::cvtColor(vis, vis, cv::COLOR_GRAY2BGR);
        }
        const int H = vis.rows;
        const int W = vis.cols;

        for (int i = 0; i < H && i < static_cast<int>(seam.size()); ++i) {
            int j = seam[i];
            if (j >= 0 && j < W) {
                vis.at<cv::Vec3b>(i, j) = cv::Vec3b(
                    static_cast<uchar>(color[0]),
                    static_cast<uchar>(color[1]),
                    static_cast<uchar>(color[2])
                );
            }
        }
        return vis;
    }
};

} // namespace chronocarve
