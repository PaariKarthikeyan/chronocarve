#pragma once

#include <opencv2/opencv.hpp>
#include <string>
#include "common.hpp"

namespace chronocarve {

class ImageIO {
public:
    static bool load_image(const std::string& path, cv::Mat& out_image);
    static bool save_image(const std::string& path, const cv::Mat& image, int quality = 95);
    static bool get_image_metadata(const std::string& path, ImageMetadata& out_meta);

    // Generates a rich high-detail synthetic landscape image for immediate testing
    static bool generate_sample_landscape(
        const std::string& output_path,
        int width = 960,
        int height = 540,
        int style_id = 0 // 0: Mountain Lake, 1: Sunset Horizon, 2: Cyberpunk Cityscape
    );
};

} // namespace chronocarve
