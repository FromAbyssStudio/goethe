#include "goethe/statistics.hpp"
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <cmath>

namespace goethe {

// OperationStats methods
double OperationStats::compression_ratio() const {
    if (input_size == 0) return 0.0;
    return static_cast<double>(output_size) / static_cast<double>(input_size);
}

double OperationStats::compression_rate() const {
    return (1.0 - compression_ratio()) * 100.0;
}

double OperationStats::throughput_mbps() const {
    if (duration.count() == 0) return 0.0;
    double seconds = static_cast<double>(duration.count()) / 1e9;
    double mb = static_cast<double>(input_size) / (1024.0 * 1024.0);
    return mb / seconds;
}

double OperationStats::throughput_mibps() const {
    if (duration.count() == 0) return 0.0;
    double seconds = static_cast<double>(duration.count()) / 1e9;
    double mib = static_cast<double>(input_size) / (1024.0 * 1024.0);
    return mib / seconds;
}

// BackendStats copy constructor and assignment operator
BackendStats::BackendStats(const BackendStats& other) 
    : backend_name(other.backend_name)
    , backend_version(other.backend_version) {
    total_compressions.store(other.total_compressions.load());
    total_decompressions.store(other.total_decompressions.load());
    successful_compressions.store(other.successful_compressions.load());
    successful_decompressions.store(other.successful_decompressions.load());
    failed_compressions.store(other.failed_compressions.load());
    failed_decompressions.store(other.failed_decompressions.load());
    total_input_size.store(other.total_input_size.load());
    total_output_size.store(other.total_output_size.load());
    total_compressed_size.store(other.total_compressed_size.load());
    total_decompressed_size.store(other.total_decompressed_size.load());
    total_compression_time_ns.store(other.total_compression_time_ns.load());
    total_decompression_time_ns.store(other.total_decompression_time_ns.load());
}

BackendStats& BackendStats::operator=(const BackendStats& other) {
    if (this != &other) {
        backend_name = other.backend_name;
        backend_version = other.backend_version;
        total_compressions.store(other.total_compressions.load());
        total_decompressions.store(other.total_decompressions.load());
        successful_compressions.store(other.successful_compressions.load());
        successful_decompressions.store(other.successful_decompressions.load());
        failed_compressions.store(other.failed_compressions.load());
        failed_decompressions.store(other.failed_decompressions.load());
        total_input_size.store(other.total_input_size.load());
        total_output_size.store(other.total_output_size.load());
        total_compressed_size.store(other.total_compressed_size.load());
        total_decompressed_size.store(other.total_decompressed_size.load());
        total_compression_time_ns.store(other.total_compression_time_ns.load());
        total_decompression_time_ns.store(other.total_decompression_time_ns.load());
    }
    return *this;
}

// BackendStats methods
double BackendStats::average_compression_ratio() const {
    std::uint64_t total_ops = successful_compressions.load();
    if (total_ops == 0) return 0.0;
    
    std::uint64_t input = total_input_size.load();
    std::uint64_t output = total_compressed_size.load();
    
    if (input == 0) return 0.0;
    return static_cast<double>(output) / static_cast<double>(input);
}

double BackendStats::average_compression_rate() const {
    return (1.0 - average_compression_ratio()) * 100.0;
}

double BackendStats::average_compression_throughput_mbps() const {
    std::uint64_t total_ops = successful_compressions.load();
    if (total_ops == 0) return 0.0;
    
    std::uint64_t total_time_ns = total_compression_time_ns.load();
    if (total_time_ns == 0) return 0.0;
    
    double total_seconds = static_cast<double>(total_time_ns) / 1e9;
    double total_mb = static_cast<double>(total_input_size.load()) / (1024.0 * 1024.0);
    
    return total_mb / total_seconds;
}

double BackendStats::average_decompression_throughput_mbps() const {
    std::uint64_t total_ops = successful_decompressions.load();
    if (total_ops == 0) return 0.0;
    
    std::uint64_t total_time_ns = total_decompression_time_ns.load();
    if (total_time_ns == 0) return 0.0;
    
    double total_seconds = static_cast<double>(total_time_ns) / 1e9;
    double total_mb = static_cast<double>(total_decompressed_size.load()) / (1024.0 * 1024.0);
    
    return total_mb / total_seconds;
}

double BackendStats::success_rate() const {
    std::uint64_t total_ops = total_compressions.load() + total_decompressions.load();
    if (total_ops == 0) return 100.0;
    
    std::uint64_t successful_ops = successful_compressions.load() + successful_decompressions.load();
    return (static_cast<double>(successful_ops) / static_cast<double>(total_ops)) * 100.0;
}

void BackendStats::reset() {
    total_compressions.store(0);
    total_decompressions.store(0);
    successful_compressions.store(0);
    successful_decompressions.store(0);
    failed_compressions.store(0);
    failed_decompressions.store(0);
    total_input_size.store(0);
    total_output_size.store(0);
    total_compressed_size.store(0);
    total_decompressed_size.store(0);
    total_compression_time_ns.store(0);
    total_decompression_time_ns.store(0);
}

// StatisticsManager methods
StatisticsManager& StatisticsManager::instance() {
    static StatisticsManager instance;
    return instance;
}

void StatisticsManager::enable_statistics(bool enable) {
    std::lock_guard<std::mutex> lock(mutex_);
    enabled_ = enable;
}

bool StatisticsManager::is_statistics_enabled() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return enabled_;
}

void StatisticsManager::record_compression(const std::string& backend_name, const std::string& backend_version,
                                         const OperationStats& stats) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!enabled_) return;
    
    auto& backend_stats = backend_stats_[backend_name];
    backend_stats.backend_name = backend_name;
    backend_stats.backend_version = backend_version;
    
    backend_stats.total_compressions.fetch_add(1);
    backend_stats.total_input_size.fetch_add(stats.input_size);
    backend_stats.total_output_size.fetch_add(stats.output_size);
    backend_stats.total_compression_time_ns.fetch_add(stats.duration.count());
    
    if (stats.success) {
        backend_stats.successful_compressions.fetch_add(1);
        backend_stats.total_compressed_size.fetch_add(stats.output_size);
    } else {
        backend_stats.failed_compressions.fetch_add(1);
    }
    
    // Update global stats
    global_stats_.total_compressions.fetch_add(1);
    global_stats_.total_input_size.fetch_add(stats.input_size);
    global_stats_.total_output_size.fetch_add(stats.output_size);
    global_stats_.total_compression_time_ns.fetch_add(stats.duration.count());
    
    if (stats.success) {
        global_stats_.successful_compressions.fetch_add(1);
        global_stats_.total_compressed_size.fetch_add(stats.output_size);
    } else {
        global_stats_.failed_compressions.fetch_add(1);
    }
}

void StatisticsManager::record_decompression(const std::string& backend_name, const std::string& backend_version,
                                           const OperationStats& stats) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!enabled_) return;
    
    auto& backend_stats = backend_stats_[backend_name];
    backend_stats.backend_name = backend_name;
    backend_stats.backend_version = backend_version;
    
    backend_stats.total_decompressions.fetch_add(1);
    backend_stats.total_input_size.fetch_add(stats.input_size);
    backend_stats.total_output_size.fetch_add(stats.output_size);
    backend_stats.total_decompression_time_ns.fetch_add(stats.duration.count());
    
    if (stats.success) {
        backend_stats.successful_decompressions.fetch_add(1);
        backend_stats.total_decompressed_size.fetch_add(stats.output_size);
    } else {
        backend_stats.failed_decompressions.fetch_add(1);
    }
    
    // Update global stats
    global_stats_.total_decompressions.fetch_add(1);
    global_stats_.total_input_size.fetch_add(stats.input_size);
    global_stats_.total_output_size.fetch_add(stats.output_size);
    global_stats_.total_decompression_time_ns.fetch_add(stats.duration.count());
    
    if (stats.success) {
        global_stats_.successful_decompressions.fetch_add(1);
        global_stats_.total_decompressed_size.fetch_add(stats.output_size);
    } else {
        global_stats_.failed_decompressions.fetch_add(1);
    }
}

BackendStats StatisticsManager::get_backend_stats(const std::string& backend_name) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = backend_stats_.find(backend_name);
    if (it != backend_stats_.end()) {
        return it->second;
    }
    return BackendStats{};
}

std::vector<std::string> StatisticsManager::get_backend_names() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> names;
    names.reserve(backend_stats_.size());
    for (const auto& [name, _] : backend_stats_) {
        names.push_back(name);
    }
    return names;
}

BackendStats StatisticsManager::get_global_stats() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return global_stats_;
}

void StatisticsManager::reset_backend_stats(const std::string& backend_name) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = backend_stats_.find(backend_name);
    if (it != backend_stats_.end()) {
        it->second.reset();
    }
}

void StatisticsManager::reset_all_stats() {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& [_, stats] : backend_stats_) {
        stats.reset();
    }
    global_stats_.reset();
}

std::string StatisticsManager::export_json() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    
    oss << "{\n";
    oss << "  \"statistics_enabled\": " << (enabled_ ? "true" : "false") << ",\n";
    oss << "  \"global_stats\": {\n";
    oss << "    \"total_compressions\": " << global_stats_.total_compressions.load() << ",\n";
    oss << "    \"total_decompressions\": " << global_stats_.total_decompressions.load() << ",\n";
    oss << "    \"successful_compressions\": " << global_stats_.successful_compressions.load() << ",\n";
    oss << "    \"successful_decompressions\": " << global_stats_.successful_decompressions.load() << ",\n";
    oss << "    \"failed_compressions\": " << global_stats_.failed_compressions.load() << ",\n";
    oss << "    \"failed_decompressions\": " << global_stats_.failed_decompressions.load() << ",\n";
    oss << "    \"total_input_size\": " << global_stats_.total_input_size.load() << ",\n";
    oss << "    \"total_output_size\": " << global_stats_.total_output_size.load() << ",\n";
    oss << "    \"total_compressed_size\": " << global_stats_.total_compressed_size.load() << ",\n";
    oss << "    \"total_decompressed_size\": " << global_stats_.total_decompressed_size.load() << ",\n";
    oss << "    \"total_compression_time_ns\": " << global_stats_.total_compression_time_ns.load() << ",\n";
    oss << "    \"total_decompression_time_ns\": " << global_stats_.total_decompression_time_ns.load() << ",\n";
    oss << "    \"average_compression_ratio\": " << global_stats_.average_compression_ratio() << ",\n";
    oss << "    \"average_compression_rate\": " << global_stats_.average_compression_rate() << ",\n";
    oss << "    \"average_compression_throughput_mbps\": " << global_stats_.average_compression_throughput_mbps() << ",\n";
    oss << "    \"average_decompression_throughput_mbps\": " << global_stats_.average_decompression_throughput_mbps() << ",\n";
    oss << "    \"success_rate\": " << global_stats_.success_rate() << "\n";
    oss << "  },\n";
    oss << "  \"backend_stats\": {\n";
    
    bool first_backend = true;
    for (const auto& [name, stats] : backend_stats_) {
        if (!first_backend) oss << ",\n";
        first_backend = false;
        
        oss << "    \"" << name << "\": {\n";
        oss << "      \"backend_name\": \"" << stats.backend_name << "\",\n";
        oss << "      \"backend_version\": \"" << stats.backend_version << "\",\n";
        oss << "      \"total_compressions\": " << stats.total_compressions.load() << ",\n";
        oss << "      \"total_decompressions\": " << stats.total_decompressions.load() << ",\n";
        oss << "      \"successful_compressions\": " << stats.successful_compressions.load() << ",\n";
        oss << "      \"successful_decompressions\": " << stats.successful_decompressions.load() << ",\n";
        oss << "      \"failed_compressions\": " << stats.failed_compressions.load() << ",\n";
        oss << "      \"failed_decompressions\": " << stats.failed_decompressions.load() << ",\n";
        oss << "      \"total_input_size\": " << stats.total_input_size.load() << ",\n";
        oss << "      \"total_output_size\": " << stats.total_output_size.load() << ",\n";
        oss << "      \"total_compressed_size\": " << stats.total_compressed_size.load() << ",\n";
        oss << "      \"total_decompressed_size\": " << stats.total_decompressed_size.load() << ",\n";
        oss << "      \"total_compression_time_ns\": " << stats.total_compression_time_ns.load() << ",\n";
        oss << "      \"total_decompression_time_ns\": " << stats.total_decompression_time_ns.load() << ",\n";
        oss << "      \"average_compression_ratio\": " << stats.average_compression_ratio() << ",\n";
        oss << "      \"average_compression_rate\": " << stats.average_compression_rate() << ",\n";
        oss << "      \"average_compression_throughput_mbps\": " << stats.average_compression_throughput_mbps() << ",\n";
        oss << "      \"average_decompression_throughput_mbps\": " << stats.average_decompression_throughput_mbps() << ",\n";
        oss << "      \"success_rate\": " << stats.success_rate() << "\n";
        oss << "    }";
    }
    
    oss << "\n  }\n";
    oss << "}";
    
    return oss.str();
}

std::string StatisticsManager::export_csv() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    
    // Header
    oss << "Backend,Version,Total_Compressions,Total_Decompressions,Successful_Compressions,"
        << "Successful_Decompressions,Failed_Compressions,Failed_Decompressions,"
        << "Total_Input_Size,Total_Output_Size,Total_Compressed_Size,Total_Decompressed_Size,"
        << "Total_Compression_Time_ns,Total_Decompression_Time_ns,"
        << "Average_Compression_Ratio,Average_Compression_Rate,"
        << "Average_Compression_Throughput_MBps,Average_Decompression_Throughput_MBps,Success_Rate\n";
    
    // Global stats
    oss << "GLOBAL,," << global_stats_.total_compressions.load() << ","
        << global_stats_.total_decompressions.load() << ","
        << global_stats_.successful_compressions.load() << ","
        << global_stats_.successful_decompressions.load() << ","
        << global_stats_.failed_compressions.load() << ","
        << global_stats_.failed_decompressions.load() << ","
        << global_stats_.total_input_size.load() << ","
        << global_stats_.total_output_size.load() << ","
        << global_stats_.total_compressed_size.load() << ","
        << global_stats_.total_decompressed_size.load() << ","
        << global_stats_.total_compression_time_ns.load() << ","
        << global_stats_.total_decompression_time_ns.load() << ","
        << global_stats_.average_compression_ratio() << ","
        << global_stats_.average_compression_rate() << ","
        << global_stats_.average_compression_throughput_mbps() << ","
        << global_stats_.average_decompression_throughput_mbps() << ","
        << global_stats_.success_rate() << "\n";
    
    // Backend stats
    for (const auto& [name, stats] : backend_stats_) {
        oss << "\"" << stats.backend_name << "\",\"" << stats.backend_version << "\","
            << stats.total_compressions.load() << ","
            << stats.total_decompressions.load() << ","
            << stats.successful_compressions.load() << ","
            << stats.successful_decompressions.load() << ","
            << stats.failed_compressions.load() << ","
            << stats.failed_decompressions.load() << ","
            << stats.total_input_size.load() << ","
            << stats.total_output_size.load() << ","
            << stats.total_compressed_size.load() << ","
            << stats.total_decompressed_size.load() << ","
            << stats.total_compression_time_ns.load() << ","
            << stats.total_decompression_time_ns.load() << ","
            << stats.average_compression_ratio() << ","
            << stats.average_compression_rate() << ","
            << stats.average_compression_throughput_mbps() << ","
            << stats.average_decompression_throughput_mbps() << ","
            << stats.success_rate() << "\n";
    }
    
    return oss.str();
}

// Timer methods
StatisticsManager::Timer::Timer() : running_(false) {}

void StatisticsManager::Timer::start() {
    start_time_ = Clock::now();
    running_ = true;
}

Duration StatisticsManager::Timer::stop() {
    if (!running_) return Duration{0};
    running_ = false;
    return elapsed();
}

Duration StatisticsManager::Timer::elapsed() const {
    if (!running_) return Duration{0};
    return Clock::now() - start_time_;
}

bool StatisticsManager::Timer::is_running() const {
    return running_;
}

// StatisticsScope methods
StatisticsScope::StatisticsScope(const std::string& backend_name, const std::string& backend_version, bool is_compression)
    : backend_name_(backend_name), backend_version_(backend_version), is_compression_(is_compression) {
    timer_.start();
}

StatisticsScope::~StatisticsScope() {
    if (!recorded_) {
        set_success(false, "Operation not completed");
    }
}

void StatisticsScope::set_sizes(std::size_t input_size, std::size_t output_size) {
    input_size_ = input_size;
    output_size_ = output_size;
}

void StatisticsScope::set_success(bool success, const std::string& error_message) {
    if (recorded_) return;
    
    success_ = success;
    error_message_ = error_message;
    
    auto& stats_manager = StatisticsManager::instance();
    auto stats = create_operation_stats(input_size_, output_size_, timer_, success, error_message);
    
    if (is_compression_) {
        stats_manager.record_compression(backend_name_, backend_version_, stats);
    } else {
        stats_manager.record_decompression(backend_name_, backend_version_, stats);
    }
    
    recorded_ = true;
}

} // namespace goethe
