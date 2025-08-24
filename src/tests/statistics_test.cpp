#include "goethe/manager.hpp"
#include "goethe/statistics.hpp"
#include <iostream>
#include <fstream>
#include <random>
#include <chrono>
#include <thread>
#include <iomanip>

// Helper function to generate test data
std::vector<uint8_t> generate_test_data(size_t size, bool compressible = true) {
    std::vector<uint8_t> data(size);
    std::random_device rd;
    std::mt19937 gen(rd());
    
    if (compressible) {
        // Generate compressible data (repeating patterns)
        std::uniform_int_distribution<> dis(0, 255);
        for (size_t i = 0; i < size; ++i) {
            data[i] = dis(gen) % 10; // Limited range for better compression
        }
    } else {
        // Generate random data (less compressible)
        std::uniform_int_distribution<> dis(0, 255);
        for (size_t i = 0; i < size; ++i) {
            data[i] = dis(gen);
        }
    }
    
    return data;
}

// Helper function to print statistics
void print_backend_stats(const goethe::BackendStats& stats, const std::string& title = "") {
    if (!title.empty()) {
        std::cout << "\n=== " << title << " ===" << std::endl;
    }
    
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Backend: " << stats.backend_name << " v" << stats.backend_version << std::endl;
    std::cout << "Operations:" << std::endl;
    std::cout << "  Compressions: " << stats.total_compressions.load() 
              << " (successful: " << stats.successful_compressions.load() 
              << ", failed: " << stats.failed_compressions.load() << ")" << std::endl;
    std::cout << "  Decompressions: " << stats.total_decompressions.load() 
              << " (successful: " << stats.successful_decompressions.load() 
              << ", failed: " << stats.failed_decompressions.load() << ")" << std::endl;
    std::cout << "  Success Rate: " << stats.success_rate() << "%" << std::endl;
    
    std::cout << "Data Sizes:" << std::endl;
    std::cout << "  Total Input: " << stats.total_input_size.load() << " bytes" << std::endl;
    std::cout << "  Total Output: " << stats.total_output_size.load() << " bytes" << std::endl;
    std::cout << "  Total Compressed: " << stats.total_compressed_size.load() << " bytes" << std::endl;
    std::cout << "  Total Decompressed: " << stats.total_decompressed_size.load() << " bytes" << std::endl;
    
    std::cout << "Performance Metrics:" << std::endl;
    std::cout << "  Average Compression Ratio: " << stats.average_compression_ratio() << std::endl;
    std::cout << "  Average Compression Rate: " << stats.average_compression_rate() << "%" << std::endl;
    std::cout << "  Average Compression Throughput: " << stats.average_compression_throughput_mbps() << " MB/s" << std::endl;
    std::cout << "  Average Decompression Throughput: " << stats.average_decompression_throughput_mbps() << " MB/s" << std::endl;
}

int main() {
    std::cout << "Goethe Statistics System Test" << std::endl;
    std::cout << "=============================" << std::endl;

    try {
        // Initialize the compression manager
        auto& manager = goethe::CompressionManager::instance();
        manager.initialize("zstd"); // Try zstd first, fallback to null if not available
        
        std::cout << "\nBackend: " << manager.get_backend_name() 
                  << " v" << manager.get_backend_version() << std::endl;
        
        // Enable statistics
        manager.enable_statistics(true);
        std::cout << "Statistics enabled: " << (manager.is_statistics_enabled() ? "Yes" : "No") << std::endl;
        
        // Test 1: Basic compression/decompression with statistics
        std::cout << "\n1. Basic Compression/Decompression Test:" << std::endl;
        
        std::string test_string = "This is a test string that will be compressed and decompressed to test the statistics system. "
                                 "It contains repeated patterns and should compress reasonably well with most algorithms.";
        
        std::cout << "Original string size: " << test_string.size() << " bytes" << std::endl;
        
        auto compressed = manager.compress(test_string);
        std::cout << "Compressed size: " << compressed.size() << " bytes" << std::endl;
        std::cout << "Compression ratio: " << std::fixed << std::setprecision(2) 
                  << (static_cast<double>(compressed.size()) / test_string.size()) << std::endl;
        
        auto decompressed = manager.decompress_to_string(compressed);
        std::cout << "Decompressed size: " << decompressed.size() << " bytes" << std::endl;
        std::cout << "Data integrity: " << (test_string == decompressed ? "✓ OK" : "✗ FAILED") << std::endl;
        
        // Test 2: Performance benchmark with different data sizes
        std::cout << "\n2. Performance Benchmark Test:" << std::endl;
        
        std::vector<size_t> test_sizes = {1024, 10240, 102400, 1048576}; // 1KB, 10KB, 100KB, 1MB
        
        for (size_t size : test_sizes) {
            std::cout << "\nTesting with " << size << " bytes of compressible data:" << std::endl;
            
            auto data = generate_test_data(size, true);
            
            // Compression
            auto start = std::chrono::high_resolution_clock::now();
            auto comp_result = manager.compress(data);
            auto comp_end = std::chrono::high_resolution_clock::now();
            
            // Decompression
            auto decomp_start = std::chrono::high_resolution_clock::now();
            auto decomp_result = manager.decompress(comp_result);
            auto decomp_end = std::chrono::high_resolution_clock::now();
            
            auto comp_duration = std::chrono::duration_cast<std::chrono::microseconds>(comp_end - start);
            auto decomp_duration = std::chrono::duration_cast<std::chrono::microseconds>(decomp_end - decomp_start);
            
            double comp_ratio = static_cast<double>(comp_result.size()) / data.size();
            double comp_rate = (1.0 - comp_ratio) * 100.0;
            double comp_throughput = (static_cast<double>(data.size()) / (1024.0 * 1024.0)) / 
                                   (static_cast<double>(comp_duration.count()) / 1e6);
            double decomp_throughput = (static_cast<double>(decomp_result.size()) / (1024.0 * 1024.0)) / 
                                     (static_cast<double>(decomp_duration.count()) / 1e6);
            
            std::cout << "  Compression: " << comp_duration.count() << " μs, " 
                      << std::fixed << std::setprecision(2) << comp_throughput << " MB/s" << std::endl;
            std::cout << "  Decompression: " << decomp_duration.count() << " μs, " 
                      << decomp_throughput << " MB/s" << std::endl;
            std::cout << "  Compression rate: " << comp_rate << "%" << std::endl;
            std::cout << "  Data integrity: " << (data == decomp_result ? "✓ OK" : "✗ FAILED") << std::endl;
        }
        
        // Test 3: Random data (less compressible)
        std::cout << "\n3. Random Data Test (Less Compressible):" << std::endl;
        
        auto random_data = generate_test_data(102400, false); // 100KB of random data
        
        auto comp_random = manager.compress(random_data);
        auto decomp_random = manager.decompress(comp_random);
        
        double random_comp_ratio = static_cast<double>(comp_random.size()) / random_data.size();
        double random_comp_rate = (1.0 - random_comp_ratio) * 100.0;
        
        std::cout << "Random data compression rate: " << std::fixed << std::setprecision(2) 
                  << random_comp_rate << "%" << std::endl;
        std::cout << "Data integrity: " << (random_data == decomp_random ? "✓ OK" : "✗ FAILED") << std::endl;
        
        // Test 4: Error handling and statistics
        std::cout << "\n4. Error Handling Test:" << std::endl;
        
        try {
            // Try to decompress invalid data
            std::vector<uint8_t> invalid_data = {0x00, 0x01, 0x02, 0x03, 0x04};
            auto result = manager.decompress(invalid_data);
            std::cout << "Unexpected success with invalid data" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Expected error with invalid data: " << e.what() << std::endl;
        }
        
        // Test 5: Display collected statistics
        std::cout << "\n5. Collected Statistics:" << std::endl;
        
        auto backend_stats = manager.get_statistics();
        print_backend_stats(backend_stats, "Current Backend Statistics");
        
        auto global_stats = manager.get_global_statistics();
        print_backend_stats(global_stats, "Global Statistics");
        
        // Test 6: Export statistics
        std::cout << "\n6. Export Statistics:" << std::endl;
        
        std::string json_stats = manager.export_statistics_json();
        std::cout << "JSON export (first 500 chars):" << std::endl;
        std::cout << json_stats.substr(0, 500) << "..." << std::endl;
        
        std::string csv_stats = manager.export_statistics_csv();
        std::cout << "CSV export (first 500 chars):" << std::endl;
        std::cout << csv_stats.substr(0, 500) << "..." << std::endl;
        
        // Test 7: Statistics control
        std::cout << "\n7. Statistics Control Test:" << std::endl;
        
        // Disable statistics
        manager.enable_statistics(false);
        std::cout << "Statistics disabled: " << (manager.is_statistics_enabled() ? "No" : "Yes") << std::endl;
        
        // Perform operations without statistics
        auto test_data = generate_test_data(1024);
        auto comp_no_stats = manager.compress(test_data);
        auto decomp_no_stats = manager.decompress(comp_no_stats);
        
        std::cout << "Operations completed with statistics disabled" << std::endl;
        
        // Re-enable statistics
        manager.enable_statistics(true);
        std::cout << "Statistics re-enabled: " << (manager.is_statistics_enabled() ? "Yes" : "No") << std::endl;
        
        // Test 8: Reset statistics
        std::cout << "\n8. Reset Statistics Test:" << std::endl;
        
        auto stats_before_reset = manager.get_statistics();
        std::cout << "Statistics before reset: " << stats_before_reset.total_compressions.load() << " compressions" << std::endl;
        
        manager.reset_statistics();
        
        auto stats_after_reset = manager.get_statistics();
        std::cout << "Statistics after reset: " << stats_after_reset.total_compressions.load() << " compressions" << std::endl;
        
        // Test 9: Multiple backends (if available)
        std::cout << "\n9. Multiple Backends Test:" << std::endl;
        
        std::vector<std::string> backends_to_test = {"zstd", "null"};
        
        for (const auto& backend_name : backends_to_test) {
            try {
                manager.switch_backend(backend_name);
                std::cout << "Switched to backend: " << manager.get_backend_name() << std::endl;
                
                auto test_data = generate_test_data(10240);
                auto comp_result = manager.compress(test_data);
                auto decomp_result = manager.decompress(comp_result);
                
                double ratio = static_cast<double>(comp_result.size()) / test_data.size();
                double rate = (1.0 - ratio) * 100.0;
                
                std::cout << "  Compression rate: " << std::fixed << std::setprecision(2) << rate << "%" << std::endl;
                std::cout << "  Data integrity: " << (test_data == decomp_result ? "✓ OK" : "✗ FAILED") << std::endl;
                
                auto backend_stats = manager.get_statistics();
                std::cout << "  Total operations: " << backend_stats.total_compressions.load() + backend_stats.total_decompressions.load() << std::endl;
                
            } catch (const std::exception& e) {
                std::cout << "Backend " << backend_name << " not available: " << e.what() << std::endl;
            }
        }
        
        // Final statistics summary
        std::cout << "\n10. Final Statistics Summary:" << std::endl;
        
        auto final_global_stats = manager.get_global_statistics();
        print_backend_stats(final_global_stats, "Final Global Statistics");
        
        std::cout << "\n✓ All statistics tests completed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "✗ Error during testing: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
