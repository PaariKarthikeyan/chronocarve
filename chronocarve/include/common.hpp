#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <utility>
#include <memory>
#include <functional>

namespace chronocarve {

enum class AlgorithmType {
    GREEDY = 1,
    BACKWARD_DP = 2,
    FORWARD_DP = 3,
    GRAPH_CUT_2D = 4
};

inline std::string algorithm_to_string(AlgorithmType algo) {
    switch (algo) {
        case AlgorithmType::GREEDY:
            return "Algorithm 1: Greedy Baseline (O(H))";
        case AlgorithmType::BACKWARD_DP:
            return "Algorithm 2: Classic Backward-Energy DP (Avidan-Shamir 2007)";
        case AlgorithmType::FORWARD_DP:
            return "Algorithm 3: Forward-Energy DP (Rubinstein et al. 2008)";
        case AlgorithmType::GRAPH_CUT_2D:
            return "Algorithm 4: 2D Graph Cut Seam Carving (Boykov-Kolmogorov)";
        default:
            return "Unknown Algorithm";
    }
}

inline AlgorithmType string_to_algorithm(const std::string& name) {
    if (name == "greedy" || name == "1") return AlgorithmType::GREEDY;
    if (name == "backward_dp" || name == "dp_backward" || name == "2") return AlgorithmType::BACKWARD_DP;
    if (name == "forward_dp" || name == "dp_2d" || name == "dp_forward" || name == "3") return AlgorithmType::FORWARD_DP;
    if (name == "graph_cut" || name == "graph_cut_2d" || name == "4") return AlgorithmType::GRAPH_CUT_2D;
    return AlgorithmType::FORWARD_DP;
}

enum class JobStatus {
    PENDING,
    PROCESSING,
    COMPLETED,
    FAILED
};

inline std::string status_to_string(JobStatus status) {
    switch (status) {
        case JobStatus::PENDING: return "pending";
        case JobStatus::PROCESSING: return "processing";
        case JobStatus::COMPLETED: return "completed";
        case JobStatus::FAILED: return "failed";
        default: return "unknown";
    }
}

struct RetargetConfig {
    int target_width = 0;           // Target width in pixels (e.g. 9:16 target)
    int target_height = 0;          // Optional target height
    double alpha = 1.0;             // Spatial gradient weight
    bool highlight_seams = false;   // If true, render seam trails in output image
    bool generate_heatmap = true;   // If true, generate energy heatmap visualization
};

struct SeamPath {
    int seam_index = 0;
    std::vector<int> cols;          // Column indices for each row 0..H-1
    double energy_cost = 0.0;
};

struct BenchmarkMetrics {
    std::string algorithm_id;
    std::string algorithm_name;
    double runtime_ms = 0.0;
    double throughput_mpix_s = 0.0;
    double peak_memory_mb = 0.0;
    int original_width = 0;
    int original_height = 0;
    int target_width = 0;
    int target_height = 0;
    int seams_removed = 0;
    double mean_seam_energy = 0.0;
    double total_energy_removed = 0.0;
};

struct ImageMetadata {
    std::string filename;
    int width = 0;
    int height = 0;
    int channels = 3;
    double aspect_ratio = 1.778;    // width / height
    size_t file_size_bytes = 0;
    std::string format = "JPEG";
};

} // namespace chronocarve
