#include "goethe/manager.hpp"
#include "goethe/statistics.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <iomanip>

int main() {
    std::cout << "Goethe Statistics System Demo" << std::endl;
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
        
        // Test 2: Multiple operations to accumulate statistics
        std::cout << "\n2. Multiple Operations Test:" << std::endl;
        
        std::vector<std::string> test_data = {
            "Short string",
            "This is a longer string with more content to compress",
            "Very long string with lots of repeated content that should compress well. "
            "Very long string with lots of repeated content that should compress well. "
            "Very long string with lots of repeated content that should compress well. "
            "Very long string with lots of repeated content that should compress well. "
            "Very long string with lots of repeated content that should compress well. "
        };
        
        for (size_t i = 0; i < test_data.size(); ++i) {
            auto comp = manager.compress(test_data[i]);
            auto decomp = manager.decompress_to_string(comp);
            
            double ratio = static_cast<double>(comp.size()) / test_data[i].size();
            double rate = (1.0 - ratio) * 100.0;
            
            std::cout << "  Test " << (i + 1) << ": " << test_data[i].size() << " -> " 
                      << comp.size() << " bytes (" << std::fixed << std::setprecision(1) 
                      << rate << "% compression)" << std::endl;
        }
        
        // Test 3: Display collected statistics
        std::cout << "\n3. Collected Statistics:" << std::endl;
        
        auto backend_stats = manager.get_statistics();
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Backend: " << backend_stats.backend_name << " v" << backend_stats.backend_version << std::endl;
        std::cout << "Operations:" << std::endl;
        std::cout << "  Total Compressions: " << backend_stats.total_compressions.load() << std::endl;
        std::cout << "  Total Decompressions: " << backend_stats.total_decompressions.load() << std::endl;
        std::cout << "  Successful Compressions: " << backend_stats.successful_compressions.load() << std::endl;
        std::cout << "  Successful Decompressions: " << backend_stats.successful_decompressions.load() << std::endl;
        std::cout << "  Failed Compressions: " << backend_stats.failed_compressions.load() << std::endl;
        std::cout << "  Failed Decompressions: " << backend_stats.failed_decompressions.load() << std::endl;
        std::cout << "  Success Rate: " << backend_stats.success_rate() << "%" << std::endl;
        
        std::cout << "Data Sizes:" << std::endl;
        std::cout << "  Total Input: " << backend_stats.total_input_size.load() << " bytes" << std::endl;
        std::cout << "  Total Output: " << backend_stats.total_output_size.load() << " bytes" << std::endl;
        std::cout << "  Total Compressed: " << backend_stats.total_compressed_size.load() << " bytes" << std::endl;
        std::cout << "  Total Decompressed: " << backend_stats.total_decompressed_size.load() << " bytes" << std::endl;
        
        std::cout << "Performance Metrics:" << std::endl;
        std::cout << "  Average Compression Ratio: " << backend_stats.average_compression_ratio() << std::endl;
        std::cout << "  Average Compression Rate: " << backend_stats.average_compression_rate() << "%" << std::endl;
        std::cout << "  Average Compression Throughput: " << backend_stats.average_compression_throughput_mbps() << " MB/s" << std::endl;
        std::cout << "  Average Decompression Throughput: " << backend_stats.average_decompression_throughput_mbps() << " MB/s" << std::endl;
        
        // Test 4: Global statistics
        std::cout << "\n4. Global Statistics:" << std::endl;
        
        auto global_stats = manager.get_global_statistics();
        std::cout << "Global Success Rate: " << global_stats.success_rate() << "%" << std::endl;
        std::cout << "Global Average Compression Rate: " << global_stats.average_compression_rate() << "%" << std::endl;
        
        // Test 5: Export statistics
        std::cout << "\n5. Export Statistics:" << std::endl;
        
        std::string json_stats = manager.export_statistics_json();
        std::cout << "JSON export (first 300 chars):" << std::endl;
        std::cout << json_stats.substr(0, 300) << "..." << std::endl;
        
        std::string csv_stats = manager.export_statistics_csv();
        std::cout << "CSV export (first 300 chars):" << std::endl;
        std::cout << csv_stats.substr(0, 300) << "..." << std::endl;
        
        // Test 6: Statistics control
        std::cout << "\n6. Statistics Control Test:" << std::endl;
        
        // Disable statistics
        manager.enable_statistics(false);
        std::cout << "Statistics disabled: " << (manager.is_statistics_enabled() ? "No" : "Yes") << std::endl;
        
        // Perform operations without statistics
        auto test_data_no_stats = "This operation won't be tracked";
        auto comp_no_stats = manager.compress(test_data_no_stats);
        auto decomp_no_stats = manager.decompress_to_string(comp_no_stats);
        
        std::cout << "Operations completed with statistics disabled" << std::endl;
        
        // Re-enable statistics
        manager.enable_statistics(true);
        std::cout << "Statistics re-enabled: " << (manager.is_statistics_enabled() ? "Yes" : "No") << std::endl;
        
        // Test 7: Reset statistics
        std::cout << "\n7. Reset Statistics Test:" << std::endl;
        
        auto stats_before_reset = manager.get_statistics();
        std::cout << "Statistics before reset: " << stats_before_reset.total_compressions.load() << " compressions" << std::endl;
        
        manager.reset_statistics();
        
        auto stats_after_reset = manager.get_statistics();
        std::cout << "Statistics after reset: " << stats_after_reset.total_compressions.load() << " compressions" << std::endl;
        
        std::cout << "\n✓ All statistics tests completed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "✗ Error during testing: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
