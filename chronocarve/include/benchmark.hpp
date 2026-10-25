#pragma once

#include <vector>
#include <string>
#include <chrono>
#include "common.hpp"

namespace chronocarve {

class BenchmarkTracker {
public:
    BenchmarkTracker(const std::string& algo_id, const std::string& algo_name);

    void start();
    void stop();

    // Queries current process RSS/working set memory in MB
    static double get_peak_memory_mb();

    BenchmarkMetrics finalize(
        int orig_w,
        int orig_h,
        int target_w,
        int target_h,
        int seams_removed,
        const std::vector<SeamPath>& all_seams
    );

private:
    std::string algo_id_;
    std::string algo_name_;
    std::chrono::high_resolution_clock::time_point start_time_;
    std::chrono::high_resolution_clock::time_point stop_time_;
    double elapsed_ms_ = 0.0;
};

} // namespace chronocarve
