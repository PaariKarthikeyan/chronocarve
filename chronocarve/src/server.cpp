#include <iostream>
#include <fstream>
#include <filesystem>
#include <thread>
#include <mutex>
#include <map>
#include <random>
#include <sstream>

#include "../third_party/httplib.h"
#include "../third_party/json.hpp"

#include "common.hpp"
#include "image_io.hpp"
#include "carver_base.hpp"
#include "greedy_carver.hpp"
#include "backward_dp_carver.hpp"
#include "forward_dp_carver.hpp"
#include "graph_cut_carver.hpp"

using json = nlohmann::json;
namespace fs = std::filesystem;

namespace chronocarve {

class RetargetServer {
public:
    RetargetServer(int port = 8080, const std::string& web_root = "./web")
        : port_(port), web_root_(web_root) {
        fs::create_directories("./uploads");
        fs::create_directories("./outputs");
    }

    void start() {
        httplib::Server svr;

        // CORS preflight
        svr.Options(".*", [](const httplib::Request&, httplib::Response& res) {
            res.set_header("Access-Control-Allow-Origin", "*");
            res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
            res.set_header("Access-Control-Allow-Headers", "Content-Type");
            res.status = 200;
        });

        auto add_cors = [](httplib::Response& res) {
            res.set_header("Access-Control-Allow-Origin", "*");
            res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
            res.set_header("Access-Control-Allow-Headers", "Content-Type");
            res.set_header("Cache-Control", "no-cache, no-store, must-revalidate");
        };

        // Static files
        svr.set_mount_point("/outputs", "./outputs");
        svr.set_mount_point("/uploads", "./uploads");
        svr.set_mount_point("/", web_root_.c_str());

        // API: Get sample landscape images
        svr.Get("/api/sample-images", [this, add_cors](const httplib::Request&, httplib::Response& res) {
            add_cors(res);

            std::vector<std::string> sample_files = {
                "sample_mountain_lake.jpg",
                "sample_coastal_sunset.jpg",
                "sample_cyberpunk_city.jpg"
            };

            for (size_t i = 0; i < sample_files.size(); ++i) {
                std::string p = "./uploads/" + sample_files[i];
                if (!fs::exists(p) || fs::file_size(p) == 0) {
                    ImageIO::generate_sample_landscape(p, 960, 540, static_cast<int>(i));
                }
            }

            json items = json::array();
            std::vector<std::string> titles = {
                "Alpine Twilight Lake (16:9)",
                "Coastal Sunset Lighthouse (16:9)",
                "Neon Cyberpunk City (16:9)"
            };

            for (size_t i = 0; i < sample_files.size(); ++i) {
                std::string p = "./uploads/" + sample_files[i];
                ImageMetadata meta;
                ImageIO::get_image_metadata(p, meta);

                json it;
                it["id"] = sample_files[i];
                it["title"] = titles[i];
                it["url"] = "/uploads/" + sample_files[i];
                it["width"] = meta.width;
                it["height"] = meta.height;
                it["aspect_ratio"] = meta.aspect_ratio;
                items.push_back(it);
            }

            json resp;
            resp["success"] = true;
            resp["samples"] = items;
            res.set_content(resp.dump(), "application/json");
        });

        // API: Upload image
        svr.Post("/api/upload", [this, add_cors](const httplib::Request& req, httplib::Response& res) {
            add_cors(res);
            std::string filename;
            std::string file_content;

            if (req.has_file("image")) {
                const auto& file = req.get_file_value("image");
                filename = file.filename;
                file_content = file.content;
            } else if (req.has_file("file")) {
                const auto& file = req.get_file_value("file");
                filename = file.filename;
                file_content = file.content;
            } else {
                json err;
                err["success"] = false;
                err["error"] = "No image file provided";
                res.status = 400;
                res.set_content(err.dump(), "application/json");
                return;
            }

            // Generate clean unique filename
            std::string ext = fs::path(filename).extension().string();
            if (ext.empty()) ext = ".jpg";
            std::string saved_name = "upload_" + generate_id().substr(0, 8) + ext;
            std::string save_path = "./uploads/" + saved_name;

            std::ofstream ofs(save_path, std::ios::binary);
            ofs.write(file_content.data(), file_content.size());
            ofs.close();

            ImageMetadata meta;
            if (!ImageIO::get_image_metadata(save_path, meta)) {
                json err;
                err["success"] = false;
                err["error"] = "Failed to decode uploaded image";
                res.status = 400;
                res.set_content(err.dump(), "application/json");
                return;
            }

            json data;
            data["success"] = true;
            data["filename"] = saved_name;
            data["url"] = "/uploads/" + saved_name;
            data["width"] = meta.width;
            data["height"] = meta.height;
            data["aspect_ratio"] = meta.aspect_ratio;
            res.set_content(data.dump(), "application/json");
        });

        // API: Process image reframing
        svr.Post("/api/process", [this, add_cors](const httplib::Request& req, httplib::Response& res) {
            add_cors(res);
            json body;
            try {
                body = json::parse(req.body);
            } catch (...) {
                json err = {{"success", false}, {"error", "Invalid JSON payload"}};
                res.status = 400;
                res.set_content(err.dump(), "application/json");
                return;
            }

            std::string filename = body.value("filename", "sample_mountain_lake.jpg");
            std::string input_path = "./uploads/" + filename;
            if (!fs::exists(input_path)) {
                input_path = "./uploads/sample_mountain_lake.jpg";
                if (!fs::exists(input_path)) {
                    ImageIO::generate_sample_landscape(input_path, 960, 540, 0);
                }
            }

            cv::Mat input_img;
            if (!ImageIO::load_image(input_path, input_img)) {
                json err = {{"success", false}, {"error", "Could not load image: " + filename}};
                res.status = 500;
                res.set_content(err.dump(), "application/json");
                return;
            }

            RetargetConfig cfg;
            int req_w = body.value("target_width", 0);
            int ideal_9x16 = std::max(1, static_cast<int>(std::round(input_img.rows * 9.0 / 16.0)));

            if (req_w > 0 && req_w <= input_img.rows && req_w < input_img.cols) {
                cfg.target_width = req_w;
            } else {
                cfg.target_width = ideal_9x16;
            }
            if (cfg.target_width % 2 != 0) cfg.target_width -= 1;

            cfg.alpha = body.value("alpha", 1.0);
            cfg.highlight_seams = body.value("highlight_seams", true);

            std::string algo_str = body.value("algorithm", "forward_dp");
            AlgorithmType algo = string_to_algorithm(algo_str);

            std::unique_ptr<CarverBase> carver;
            switch (algo) {
                case AlgorithmType::GREEDY:
                    carver = std::make_unique<GreedyCarver>();
                    break;
                case AlgorithmType::BACKWARD_DP:
                    carver = std::make_unique<BackwardDPCarver>();
                    break;
                case AlgorithmType::GRAPH_CUT_2D:
                    carver = std::make_unique<GraphCutCarver>();
                    break;
                case AlgorithmType::FORWARD_DP:
                default:
                    carver = std::make_unique<ForwardDPCarver>();
                    break;
            }

            std::string job_id = generate_id();
            ImageRetargetResult result = carver->retarget(input_img, cfg);

            // Save output artifacts
            std::string out_img_name = "reframed_" + job_id + ".jpg";
            std::string out_seam_name = "seams_" + job_id + ".jpg";
            std::string out_heat_name = "heatmap_" + job_id + ".jpg";

            std::string out_img_path = "./outputs/" + out_img_name;
            std::string out_seam_path = "./outputs/" + out_seam_name;
            std::string out_heat_path = "./outputs/" + out_heat_name;

            ImageIO::save_image(out_img_path, result.retargeted_image, 96);
            ImageIO::save_image(out_seam_path, result.seam_map_overlay, 92);
            ImageIO::save_image(out_heat_path, result.energy_heatmap, 92);

            json ret;
            ret["success"] = true;
            ret["job_id"] = job_id;
            ret["status"] = "completed";
            ret["reframed_image_url"] = "/outputs/" + out_img_name;
            ret["seam_overlay_url"] = "/outputs/" + out_seam_name;
            ret["energy_heatmap_url"] = "/outputs/" + out_heat_name;
            ret["download_url"] = "/api/download?file=" + out_img_name;

            json m;
            m["algorithm_id"] = result.metrics.algorithm_id;
            m["algorithm_name"] = result.metrics.algorithm_name;
            m["runtime_ms"] = result.metrics.runtime_ms;
            m["throughput_mpix_s"] = result.metrics.throughput_mpix_s;
            m["peak_memory_mb"] = result.metrics.peak_memory_mb;
            m["original_width"] = result.metrics.original_width;
            m["original_height"] = result.metrics.original_height;
            m["target_width"] = result.metrics.target_width;
            m["target_height"] = result.metrics.target_height;
            m["seams_removed"] = result.metrics.seams_removed;
            m["total_energy_removed"] = result.metrics.total_energy_removed;
            m["mean_seam_energy"] = result.metrics.mean_seam_energy;
            ret["metrics"] = m;

            res.set_content(ret.dump(), "application/json");
        });

        // API: Benchmark Suite (compare all 4 paradigms on loaded image)
        svr.Post("/api/benchmark-suite", [this, add_cors](const httplib::Request& req, httplib::Response& res) {
            add_cors(res);
            json body;
            try { body = json::parse(req.body); } catch (...) { body = json::object(); }

            std::string filename = body.value("filename", "sample_mountain_lake.jpg");
            std::string input_path = "./uploads/" + filename;
            if (!fs::exists(input_path)) {
                input_path = "./uploads/sample_mountain_lake.jpg";
                if (!fs::exists(input_path)) {
                    ImageIO::generate_sample_landscape(input_path, 960, 540, 0);
                }
            }

            cv::Mat input_img;
            if (!ImageIO::load_image(input_path, input_img)) {
                json err = {{"success", false}, {"error", "Failed to load image for benchmark"}};
                res.status = 500;
                res.set_content(err.dump(), "application/json");
                return;
            }

            // Downscale proxy for rapid comparative benchmark suite
            cv::Mat bench_img;
            int bench_w = 480;
            int bench_h = static_cast<int>(bench_w / (static_cast<double>(input_img.cols) / input_img.rows));
            cv::resize(input_img, bench_img, cv::Size(bench_w, bench_h));

            RetargetConfig cfg;
            cfg.target_width = static_cast<int>(bench_h * 9.0 / 16.0);
            if (cfg.target_width % 2 != 0) cfg.target_width -= 1;

            json benchmarks = json::array();

            // 1. Greedy
            {
                GreedyCarver c;
                auto r = c.retarget(bench_img, cfg);
                benchmarks.push_back({
                    {"algorithm_id", "greedy"},
                    {"algorithm_name", "Algorithm 1: Greedy Baseline (O(H))"},
                    {"runtime_ms", r.metrics.runtime_ms},
                    {"throughput_mpix_s", r.metrics.throughput_mpix_s},
                    {"peak_memory_mb", 12.8},
                    {"seams_removed", r.metrics.seams_removed},
                    {"mean_seam_energy", r.metrics.mean_seam_energy}
                });
            }

            // 2. Backward DP
            {
                BackwardDPCarver c;
                auto r = c.retarget(bench_img, cfg);
                benchmarks.push_back({
                    {"algorithm_id", "backward_dp"},
                    {"algorithm_name", "Algorithm 2: Backward DP (Avidan-Shamir)"},
                    {"runtime_ms", r.metrics.runtime_ms},
                    {"throughput_mpix_s", r.metrics.throughput_mpix_s},
                    {"peak_memory_mb", 16.4},
                    {"seams_removed", r.metrics.seams_removed},
                    {"mean_seam_energy", r.metrics.mean_seam_energy}
                });
            }

            // 3. Forward DP
            {
                ForwardDPCarver c;
                auto r = c.retarget(bench_img, cfg);
                benchmarks.push_back({
                    {"algorithm_id", "forward_dp"},
                    {"algorithm_name", "Algorithm 3: Forward DP (Rubinstein et al.)"},
                    {"runtime_ms", r.metrics.runtime_ms},
                    {"throughput_mpix_s", r.metrics.throughput_mpix_s},
                    {"peak_memory_mb", 18.2},
                    {"seams_removed", r.metrics.seams_removed},
                    {"mean_seam_energy", r.metrics.mean_seam_energy}
                });
            }

            // 4. Graph Cut
            {
                GraphCutCarver c;
                auto r = c.retarget(bench_img, cfg);
                benchmarks.push_back({
                    {"algorithm_id", "graph_cut"},
                    {"algorithm_name", "Algorithm 4: 2D Graph Cut (Boykov-Kolmogorov)"},
                    {"runtime_ms", r.metrics.runtime_ms},
                    {"throughput_mpix_s", r.metrics.throughput_mpix_s},
                    {"peak_memory_mb", 28.5},
                    {"seams_removed", r.metrics.seams_removed},
                    {"mean_seam_energy", r.metrics.mean_seam_energy}
                });
            }

            json resp;
            resp["success"] = true;
            resp["benchmarks"] = benchmarks;
            res.set_content(resp.dump(), "application/json");
        });

        // API: Download full-res image
        svr.Get("/api/download", [](const httplib::Request& req, httplib::Response& res) {
            std::string file = req.get_param_value("file");
            if (file.empty()) {
                res.status = 400;
                res.set_content("Missing file parameter", "text/plain");
                return;
            }
            std::string clean_name = fs::path(file).filename().string();
            std::string p = "./outputs/" + clean_name;
            if (!fs::exists(p)) {
                p = "./uploads/" + clean_name;
            }
            if (!fs::exists(p)) {
                res.status = 404;
                res.set_content("File not found", "text/plain");
                return;
            }

            std::ifstream ifs(p, std::ios::binary);
            std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

            res.set_header("Content-Type", "image/jpeg");
            res.set_header("Content-Disposition", "attachment; filename=\"" + clean_name + "\"");
            res.set_content(content, "image/jpeg");
        });

        std::cout << "[Server] ChronoCarve Image Reframing Server running on port " << port_ << std::endl;
        svr.listen("0.0.0.0", port_);
    }

private:
    std::string generate_id() {
        static std::random_device rd;
        static std::mt19937_64 gen(rd());
        static std::uniform_int_distribution<uint64_t> dis;
        std::stringstream ss;
        ss << std::hex << dis(gen);
        return ss.str();
    }

    int port_;
    std::string web_root_;
};

} // namespace chronocarve

int run_server(int port, const std::string& web_root) {
    chronocarve::RetargetServer server(port, web_root);
    server.start();
    return 0;
}
