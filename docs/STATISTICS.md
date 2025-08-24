# Goethe Statistics System

The Goethe library includes a comprehensive statistics system that tracks compression performance, throughput, and other relevant metrics. This system provides detailed insights into the performance characteristics of different compression backends.

## Overview

The statistics system tracks:
- **Compression rates** - How much data is being compressed
- **Read/write velocities** - Throughput in MB/s for compression and decompression
- **Success rates** - Percentage of successful operations
- **Data sizes** - Total input/output sizes processed
- **Timing information** - Precise timing for performance analysis

## Key Components

### 1. OperationStats
Tracks statistics for individual compression/decompression operations:

```cpp
struct OperationStats {
    std::size_t input_size = 0;      // Input data size in bytes
    std::size_t output_size = 0;     // Output data size in bytes
    Duration duration{};              // Operation duration
    bool success = false;             // Whether operation succeeded
    std::string error_message;       // Error message if failed
    
    // Calculated metrics
    double compression_ratio() const;     // output_size / input_size
    double compression_rate() const;      // (1.0 - compression_ratio()) * 100.0
    double throughput_mbps() const;       // Throughput in MB/s
    double throughput_mibps() const;      // Throughput in MiB/s
};
```

### 2. BackendStats
Aggregates statistics for a specific compression backend:

```cpp
struct BackendStats {
    std::string backend_name;
    std::string backend_version;
    
    // Operation counters (atomic for thread safety)
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
    
    // Performance metrics
    double average_compression_ratio() const;
    double average_compression_rate() const;
    double average_compression_throughput_mbps() const;
    double average_decompression_throughput_mbps() const;
    double success_rate() const;
};
```

### 3. StatisticsManager
Global singleton that manages all statistics collection:

```cpp
class StatisticsManager {
public:
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
    BackendStats get_global_stats() const;
    
    // Reset statistics
    void reset_backend_stats(const std::string& backend_name);
    void reset_all_stats();
    
    // Export statistics
    std::string export_json() const;
    std::string export_csv() const;
};
```

## Usage Examples

### Basic Usage

```cpp
#include "goethe/manager.hpp"
#include "goethe/statistics.hpp"

// Initialize the compression manager
auto& manager = goethe::CompressionManager::instance();
manager.initialize("zstd");

// Enable statistics collection
manager.enable_statistics(true);

// Perform compression operations
std::string data = "This is test data that will be compressed";
auto compressed = manager.compress(data);
auto decompressed = manager.decompress_to_string(compressed);

// Get statistics
auto stats = manager.get_statistics();
std::cout << "Compression rate: " << stats.average_compression_rate() << "%" << std::endl;
std::cout << "Throughput: " << stats.average_compression_throughput_mbps() << " MB/s" << std::endl;
```

### Advanced Usage with Manual Statistics

```cpp
#include "goethe/statistics.hpp"

// Create a timer for manual timing
auto timer = goethe::start_timer();

// Perform your operation
auto result = compress_data(data);

// Create operation stats
auto stats = goethe::create_operation_stats(
    data.size(), 
    result.size(), 
    timer, 
    true, 
    ""
);

// Record the statistics
auto& stats_manager = goethe::StatisticsManager::instance();
stats_manager.record_compression("zstd", "1.5.2", stats);
```

### RAII Statistics Scope

```cpp
#include "goethe/statistics.hpp"

{
    // Automatically starts timing
    goethe::StatisticsScope scope("zstd", "1.5.2", true);
    
    // Perform compression
    auto result = compress_data(data);
    
    // Set sizes and mark as successful
    scope.set_sizes(data.size(), result.size());
    scope.set_success(true);
    
    // Statistics are automatically recorded when scope exits
}
```

## Performance Metrics

### Compression Rate
The percentage of data reduction achieved:
```
compression_rate = (1.0 - compressed_size / original_size) * 100.0
```

### Throughput
Data processing speed in MB/s:
```
throughput = (data_size_mb / time_seconds)
```

### Success Rate
Percentage of successful operations:
```
success_rate = (successful_operations / total_operations) * 100.0
```

## Command Line Tool

The library includes a command-line tool for statistics management:

```bash
# Show current backend information
./statistics_tool info

# Show current statistics
./statistics_tool stats

# Show global statistics
./statistics_tool global

# Enable/disable statistics collection
./statistics_tool enable
./statistics_tool disable

# Reset all statistics
./statistics_tool reset

# Export statistics to JSON
./statistics_tool export-json stats.json

# Export statistics to CSV
./statistics_tool export-csv stats.csv

# Run benchmark with 1MB data
./statistics_tool benchmark 1048576

# Run stress test with 1000 operations
./statistics_tool stress-test 1000

# Switch to different backend
./statistics_tool switch null
```

## Export Formats

### JSON Export
```json
{
  "statistics_enabled": true,
  "global_stats": {
    "total_compressions": 150,
    "total_decompressions": 150,
    "successful_compressions": 148,
    "successful_decompressions": 150,
    "failed_compressions": 2,
    "failed_decompressions": 0,
    "total_input_size": 15728640,
    "total_output_size": 3145728,
    "total_compressed_size": 3145728,
    "total_decompressed_size": 15728640,
    "total_compression_time_ns": 125000000,
    "total_decompression_time_ns": 50000000,
    "average_compression_ratio": 0.20,
    "average_compression_rate": 80.00,
    "average_compression_throughput_mbps": 125.83,
    "average_decompression_throughput_mbps": 314.57,
    "success_rate": 99.33
  },
  "backend_stats": {
    "zstd": {
      "backend_name": "zstd",
      "backend_version": "1.5.2",
      "total_compressions": 150,
      "total_decompressions": 150,
      "successful_compressions": 148,
      "successful_decompressions": 150,
      "failed_compressions": 2,
      "failed_decompressions": 0,
      "total_input_size": 15728640,
      "total_output_size": 3145728,
      "total_compressed_size": 3145728,
      "total_decompressed_size": 15728640,
      "total_compression_time_ns": 125000000,
      "total_decompression_time_ns": 50000000,
      "average_compression_ratio": 0.20,
      "average_compression_rate": 80.00,
      "average_compression_throughput_mbps": 125.83,
      "average_decompression_throughput_mbps": 314.57,
      "success_rate": 99.33
    }
  }
}
```

### CSV Export
```csv
Backend,Version,Total_Compressions,Total_Decompressions,Successful_Compressions,Successful_Decompressions,Failed_Compressions,Failed_Decompressions,Total_Input_Size,Total_Output_Size,Total_Compressed_Size,Total_Decompressed_Size,Total_Compression_Time_ns,Total_Decompression_Time_ns,Average_Compression_Ratio,Average_Compression_Rate,Average_Compression_Throughput_MBps,Average_Decompression_Throughput_MBps,Success_Rate
GLOBAL,,150,150,148,150,2,0,15728640,3145728,3145728,15728640,125000000,50000000,0.20,80.00,125.83,314.57,99.33
"zstd","1.5.2",150,150,148,150,2,0,15728640,3145728,3145728,15728640,125000000,50000000,0.20,80.00,125.83,314.57,99.33
```

## Thread Safety

The statistics system is designed to be thread-safe:
- All counters use `std::atomic` for thread-safe increments
- The `StatisticsManager` uses mutex protection for map operations
- Multiple threads can safely record statistics simultaneously

## Performance Impact

The statistics system has minimal performance impact:
- **Enabled**: ~1-2% overhead for timing and counter updates
- **Disabled**: Zero overhead (all statistics calls are no-ops)
- **Memory usage**: ~1KB per backend for statistics storage

## Best Practices

1. **Enable statistics during development/testing**: Use statistics to profile and optimize your compression usage.

2. **Disable in production if not needed**: If you don't need statistics in production, disable them to eliminate overhead.

3. **Use RAII scopes for automatic tracking**: The `StatisticsScope` class automatically handles timing and recording.

4. **Export statistics periodically**: Use the export functions to save statistics for analysis.

5. **Monitor success rates**: Keep an eye on success rates to detect compression issues.

6. **Compare backends**: Use statistics to compare performance between different compression backends.

## Integration with Existing Code

The statistics system is automatically integrated into the compression backends. When you use the `CompressionManager`, statistics are automatically collected if enabled:

```cpp
// Statistics are automatically collected for these operations
auto compressed = manager.compress(data);
auto decompressed = manager.decompress(compressed);
auto string_result = manager.decompress_to_string(compressed);
```

No changes to existing code are required to enable statistics collection.
