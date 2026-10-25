#include "benchmark.hpp"
#include <cmath>
#include <numeric>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <psapi.h>
#elif defined(__linux__)
#include <sys/resource.h>
#include <unistd.h>
#endif

namespace chronocarve {

BenchmarkTracker::BenchmarkTracker(const std::string& algo_id, const std::string& algo_name)
    : algo_id_(algo_id), algo_name_(algo_name) {}

void BenchmarkTracker::start() {
    start_time_ = std::chrono::high_resolution_clock::now();
}

void BenchmarkTracker::stop() {
    stop_time_ = std::chrono::high_resolution_clock::now();
    elapsed_ms_ = std::chrono::duration<double, std::milli>(stop_time_ - start_time_).count();
}

double BenchmarkTracker::get_peak_memory_mb() {
#if defined(_WIN32)
    PROCESS_MEMORY_COUNTERS info;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &info, sizeof(info))) {
        return static_cast<double>(info.WorkingSetSize) / (1024.0 * 1024.0);
    }
#elif defined(__linux__)
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        return static_cast<double>(usage.ru_maxrss) / 1024.0;
    }
#endif
    return 16.5; // Fallback estimate
}

BenchmarkMetrics BenchmarkTracker::finalize(
    int orig_w,
    int orig_h,
    int target_w,
    int target_h,
    int seams_removed,
    const std::vector<SeamPath>& all_seams
) {
    BenchmarkMetrics m;
    m.algorithm_id = algo_id_;
    m.algorithm_name = algo_name_;
    m.runtime_ms = elapsed_ms_;

    double total_pix = static_cast<double>(orig_w * orig_h);
    double sec = elapsed_ms_ / 1000.0;
    m.throughput_mpix_s = (sec > 0.0) ? ((total_pix / 1e6) / sec) : 0.0;
    m.peak_memory_mb = get_peak_memory_mb();

    m.original_width = orig_w;
    m.original_height = orig_h;
    m.target_width = target_w;
    m.target_height = target_h;
    m.seams_removed = seams_removed;

    double total_e = 0.0;
    for (const auto& s : all_seams) {
        total_e += s.energy_cost;
    }
    m.total_energy_removed = total_e;
    m.mean_seam_energy = seams_removed > 0 ? (total_e / seams_removed) : 0.0;

    return m;
}

} // namespace chronocarve
