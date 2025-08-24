#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <atomic>
#include <mutex>

// Include the header that defines GOETHE_API
#include "goethe/dialog.hpp"

namespace goethe {

// High-resolution clock for precise timing
using Clock = std::chrono::high_resolution_clock;
using TimePoint = Clock::time_point;
using Duration = std::chrono::nanoseconds;

// Statistics for a single operation
struct OperationStats {
    std::size_t input_size = 0;      // Input data size in bytes
    std::size_t output_size = 0;     // Output data size in bytes
    Duration duration{};              // Operation duration
    bool success = false;             // Whether operation succeeded
    std::string error_message;       // Error message if failed
    
    // Calculated metrics
    double compression_ratio() const;     // output_size / input_size (0.0 = perfect compression)
    double compression_rate() const;      // (1.0 - compression_ratio()) * 100.0
    double throughput_mbps() const;       // Throughput in MB/s
    double throughput_mibps() const;      // Throughput in MiB/s
};

// Statistics for a specific backend
struct BackendStats {
    std::string backend_name;
    std::string backend_version;
    
    // Operation counters
    std::atomic<std::uint64_t> total_compressions{0};
    std::atomic<std::uint64_t> total_decompressions{0};
    std::atomic<std::uint64_t> successful_compressions{0};
    std::atomic<std::uint64_t> successful_decompressions{0};
    std::atomic<std::uint64_t> failed_compressions{0};
    std::atomic<std::uint64_t> failed_decompressions{0};
    
    // Data size counters
    std::atomic<std::uint64_t> total_input_size{0};
    std::atomic<std::uint64_t> total_output_size{0};
    std::atomic<std::uint64_t> total_compressed_size{0};
    std::atomic<std::uint64_t> total_decompressed_size{0};
    
    // Timing
    std::atomic<std::uint64_t> total_compression_time_ns{0};
    std::atomic<std::uint64_t> total_decompression_time_ns{0};
    
    // Constructors
    BackendStats() = default;
    BackendStats(const BackendStats& other);
    BackendStats(BackendStats&& other) = default;
    BackendStats& operator=(const BackendStats& other);
    BackendStats& operator=(BackendStats&& other) = default;
    
    // Performance metrics
    GOETHE_API double average_compression_ratio() const;
    GOETHE_API double average_compression_rate() const;
    GOETHE_API double average_compression_throughput_mbps() const;
    GOETHE_API double average_decompression_throughput_mbps() const;
    GOETHE_API double success_rate() const;
    
    // Reset all statistics
    GOETHE_API void reset();
};

// Global statistics manager
class StatisticsManager {
public:
    // Singleton pattern
    static StatisticsManager& instance();
    
    // Enable/disable statistics collection
    void enable_statistics(bool enable = true);
    bool is_statistics_enabled() const;
    
    // Record operations
    void record_compression(const std::string& backend_name, const std::string& backend_version,
                          const OperationStats& stats);
    void record_decompression(const std::string& backend_name, const std::string& backend_version,
                            const OperationStats& stats);
    
    // Get statistics
    BackendStats get_backend_stats(const std::string& backend_name) const;
    std::vector<std::string> get_backend_names() const;
    
    // Get global statistics
    BackendStats get_global_stats() const;
    
    // Reset statistics
    void reset_backend_stats(const std::string& backend_name);
    void reset_all_stats();
    
    // Export statistics
    std::string export_json() const;
    std::string export_csv() const;
    
    // Utility methods for timing
    class Timer {
    public:
        Timer();
        ~Timer() = default;
        Timer(const Timer&) = delete;
        Timer(Timer&&) = default;
        Timer& operator=(const Timer&) = delete;
        Timer& operator=(Timer&&) = default;
        
        void start();
        Duration stop();
        Duration elapsed() const;
        bool is_running() const;
        
    private:
        TimePoint start_time_;
        bool running_ = false;
    };

private:
    StatisticsManager() = default;
    ~StatisticsManager() = default;
    StatisticsManager(const StatisticsManager&) = delete;
    StatisticsManager& operator=(const StatisticsManager&) = delete;
    
    mutable std::mutex mutex_;
    bool enabled_ = true;
    std::unordered_map<std::string, BackendStats> backend_stats_;
    BackendStats global_stats_;
};

// Convenience functions
inline StatisticsManager::Timer start_timer() {
    StatisticsManager::Timer timer;
    timer.start();
    return timer;
}

inline OperationStats create_operation_stats(std::size_t input_size, std::size_t output_size,
                                           const StatisticsManager::Timer& timer, bool success = true,
                                           const std::string& error_message = "") {
    OperationStats stats;
    stats.input_size = input_size;
    stats.output_size = output_size;
    stats.duration = timer.elapsed();
    stats.success = success;
    stats.error_message = error_message;
    return stats;
}

// RAII wrapper for automatic statistics recording
class StatisticsScope {
public:
    StatisticsScope(const std::string& backend_name, const std::string& backend_version, bool is_compression);
    ~StatisticsScope();
    
    void set_sizes(std::size_t input_size, std::size_t output_size);
    void set_success(bool success, const std::string& error_message = "");
    
private:
    std::string backend_name_;
    std::string backend_version_;
    bool is_compression_;
    StatisticsManager::Timer timer_;
    std::size_t input_size_ = 0;
    std::size_t output_size_ = 0;
    bool success_ = true;
    std::string error_message_;
    bool recorded_ = false;
};

} // namespace goethe
