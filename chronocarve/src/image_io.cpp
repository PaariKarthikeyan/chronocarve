#include "image_io.hpp"
#include <filesystem>
#include <cmath>
#include <iostream>

namespace fs = std::filesystem;

namespace chronocarve {

bool ImageIO::load_image(const std::string& path, cv::Mat& out_image) {
    if (!fs::exists(path)) {
        return false;
    }
    out_image = cv::imread(path, cv::IMREAD_COLOR);
    return !out_image.empty();
}

bool ImageIO::save_image(const std::string& path, const cv::Mat& image, int quality) {
    if (image.empty()) return false;
    std::vector<int> params;
    std::string ext = fs::path(path).extension().string();
    if (ext == ".jpg" || ext == ".jpeg" || ext == ".JPG") {
        params.push_back(cv::IMWRITE_JPEG_QUALITY);
        params.push_back(std::clamp(quality, 1, 100));
    } else if (ext == ".png" || ext == ".PNG") {
        params.push_back(cv::IMWRITE_PNG_COMPRESSION);
        params.push_back(3); // Fast png compression
    } else if (ext == ".webp" || ext == ".WEBP") {
        params.push_back(cv::IMWRITE_WEBP_QUALITY);
        params.push_back(std::clamp(quality, 1, 100));
    }
    return cv::imwrite(path, image, params);
}

bool ImageIO::get_image_metadata(const std::string& path, ImageMetadata& out_meta) {
    cv::Mat img = cv::imread(path, cv::IMREAD_COLOR);
    if (img.empty()) return false;

    out_meta.filename = fs::path(path).filename().string();
    out_meta.width = img.cols;
    out_meta.height = img.rows;
    out_meta.channels = img.channels();
    out_meta.aspect_ratio = static_cast<double>(img.cols) / static_cast<double>(img.rows);
    out_meta.file_size_bytes = fs::file_size(path);
    out_meta.format = fs::path(path).extension().string();
    return true;
}

bool ImageIO::generate_sample_landscape(
    const std::string& output_path,
    int W,
    int H,
    int style_id
) {
    cv::Mat img(H, W, CV_8UC3);

    if (style_id == 0) {
        // Mountain Lake at Twilight
        for (int y = 0; y < H; ++y) {
            double r_y = static_cast<double>(y) / H;
            for (int x = 0; x < W; ++x) {
                if (r_y < 0.55) {
                    // Sky gradient: Deep twilight indigo to warm peach
                    double sky_t = r_y / 0.55;
                    uchar b = static_cast<uchar>((1.0 - sky_t) * 90 + sky_t * 30);
                    uchar g = static_cast<uchar>((1.0 - sky_t) * 40 + sky_t * 80);
                    uchar r = static_cast<uchar>((1.0 - sky_t) * 20 + sky_t * 190);
                    img.at<cv::Vec3b>(y, x) = cv::Vec3b(b, g, r);
                } else {
                    // Calm alpine lake reflection
                    double lake_t = (r_y - 0.55) / 0.45;
                    uchar b = static_cast<uchar>(60 - lake_t * 25);
                    uchar g = static_cast<uchar>(45 - lake_t * 20);
                    uchar r = static_cast<uchar>(30 - lake_t * 15);
                    img.at<cv::Vec3b>(y, x) = cv::Vec3b(b, g, r);
                }
            }
        }

        // Sun / Moon disc
        int cx = static_cast<int>(W * 0.72);
        int cy = static_cast<int>(H * 0.35);
        cv::circle(img, cv::Point(cx, cy), 38, cv::Scalar(180, 220, 255), -1, cv::LINE_AA);
        cv::circle(img, cv::Point(cx, cy), 46, cv::Scalar(140, 180, 240), 2, cv::LINE_AA);

        // Distant Mountain Ridge
        std::vector<cv::Point> ridge;
        ridge.push_back(cv::Point(0, H));
        ridge.push_back(cv::Point(0, static_cast<int>(H * 0.52)));
        for (int x = 0; x <= W; x += 15) {
            double nx = static_cast<double>(x) / W;
            int peak_y = static_cast<int>(H * (0.38 + 0.12 * std::sin(nx * 7.5) + 0.08 * std::cos(nx * 14.0)));
            ridge.push_back(cv::Point(x, peak_y));
        }
        ridge.push_back(cv::Point(W, H));
        std::vector<std::vector<cv::Point>> pts{ridge};
        cv::fillPoly(img, pts, cv::Scalar(55, 35, 45), cv::LINE_AA);

        // Foreground Pine Forest on Left
        std::vector<cv::Point> hill;
        hill.push_back(cv::Point(0, H));
        hill.push_back(cv::Point(0, static_cast<int>(H * 0.45)));
        hill.push_back(cv::Point(static_cast<int>(W * 0.42), static_cast<int>(H * 0.75)));
        hill.push_back(cv::Point(0, H));
        cv::fillPoly(img, std::vector<std::vector<cv::Point>>{hill}, cv::Scalar(25, 20, 20), cv::LINE_AA);

    } else if (style_id == 1) {
        // Coastal Sunset & Lighthouse
        for (int y = 0; y < H; ++y) {
            double r_y = static_cast<double>(y) / H;
            for (int x = 0; x < W; ++x) {
                if (r_y < 0.6) {
                    double t = r_y / 0.6;
                    uchar b = static_cast<uchar>((1.0 - t) * 60 + t * 20);
                    uchar g = static_cast<uchar>((1.0 - t) * 30 + t * 120);
                    uchar r = static_cast<uchar>((1.0 - t) * 10 + t * 240);
                    img.at<cv::Vec3b>(y, x) = cv::Vec3b(b, g, r);
                } else {
                    double t = (r_y - 0.6) / 0.4;
                    uchar b = static_cast<uchar>(80 + t * 40);
                    uchar g = static_cast<uchar>(40 + t * 20);
                    uchar r = static_cast<uchar>(20 + t * 10);
                    img.at<cv::Vec3b>(y, x) = cv::Vec3b(b, g, r);
                }
            }
        }
        // Sun on horizon
        cv::circle(img, cv::Point(static_cast<int>(W * 0.4), static_cast<int>(H * 0.58)), 50, cv::Scalar(100, 210, 255), -1, cv::LINE_AA);
        // Lighthouse pillar on right
        int lx = static_cast<int>(W * 0.78);
        cv::rectangle(img, cv::Point(lx - 22, static_cast<int>(H * 0.28)), cv::Point(lx + 22, static_cast<int>(H * 0.70)), cv::Scalar(210, 210, 220), -1);
        cv::rectangle(img, cv::Point(lx - 16, static_cast<int>(H * 0.40)), cv::Point(lx + 16, static_cast<int>(H * 0.48)), cv::Scalar(40, 40, 210), -1);
        // Beacon light
        cv::circle(img, cv::Point(lx, static_cast<int>(H * 0.26)), 12, cv::Scalar(120, 255, 255), -1, cv::LINE_AA);

    } else {
        // Futuristic Cyberpunk Skyline
        for (int y = 0; y < H; ++y) {
            double r_y = static_cast<double>(y) / H;
            for (int x = 0; x < W; ++x) {
                uchar b = static_cast<uchar>(30 + r_y * 40);
                uchar g = static_cast<uchar>(10 + r_y * 15);
                uchar r = static_cast<uchar>(15 + r_y * 25);
                img.at<cv::Vec3b>(y, x) = cv::Vec3b(b, g, r);
            }
        }
        // Neon grid lines
        for (int y = static_cast<int>(H * 0.7); y < H; y += 18) {
            cv::line(img, cv::Point(0, y), cv::Point(W, y), cv::Scalar(180, 80, 180), 1, cv::LINE_AA);
        }
        // High-rise buildings
        for (int bx = 40; bx < W - 60; bx += 75) {
            int bh = static_cast<int>(H * (0.35 + 0.25 * std::sin(bx * 0.05)));
            cv::rectangle(img, cv::Point(bx, H - bh), cv::Point(bx + 55, H), cv::Scalar(45, 30, 40), -1);
            cv::rectangle(img, cv::Point(bx, H - bh), cv::Point(bx + 55, H), cv::Scalar(220, 100, 200), 1);
            // Window lights
            for (int wy = H - bh + 15; wy < H - 20; wy += 22) {
                cv::circle(img, cv::Point(bx + 18, wy), 3, cv::Scalar(240, 220, 80), -1);
                cv::circle(img, cv::Point(bx + 38, wy), 3, cv::Scalar(100, 240, 240), -1);
            }
        }
    }

    return save_image(output_path, img);
}

} // namespace chronocarve
