#include "goethe/statistics.hpp"
#include <iostream>
#include <string>

int main() {
    std::cout << "Goethe Statistics System - Minimal Test" << std::endl;
    std::cout << "=======================================" << std::endl;

    try {
        // Test the StatisticsManager singleton
        auto& stats_manager = goethe::StatisticsManager::instance();
        
        std::cout << "Statistics manager created successfully" << std::endl;
        std::cout << "Statistics enabled: " << (stats_manager.is_statistics_enabled() ? "Yes" : "No") << std::endl;
        
        // Test enabling/disabling statistics
        stats_manager.enable_statistics(false);
        std::cout << "Statistics disabled: " << (stats_manager.is_statistics_enabled() ? "No" : "Yes") << std::endl;
        
        stats_manager.enable_statistics(true);
        std::cout << "Statistics re-enabled: " << (stats_manager.is_statistics_enabled() ? "Yes" : "No") << std::endl;
        
        // Test creating operation stats
        goethe::StatisticsManager::Timer timer;
        timer.start();
        
        // Simulate some work
        std::string test_data = "This is test data for compression";
        std::string compressed_data = "Compressed data";
        
        timer.stop();
        
        auto stats = goethe::create_operation_stats(
            test_data.size(),
            compressed_data.size(),
            timer,
            true,
            ""
        );
        
        std::cout << "Operation stats created successfully" << std::endl;
        std::cout << "Input size: " << stats.input_size << " bytes" << std::endl;
        std::cout << "Output size: " << stats.output_size << " bytes" << std::endl;
        std::cout << "Duration: " << stats.duration.count() << " nanoseconds" << std::endl;
        std::cout << "Success: " << (stats.success ? "Yes" : "No") << std::endl;
        std::cout << "Compression ratio: " << stats.compression_ratio() << std::endl;
        std::cout << "Compression rate: " << stats.compression_rate() << "%" << std::endl;
        std::cout << "Throughput: " << stats.throughput_mbps() << " MB/s" << std::endl;
        
        // Test recording statistics
        stats_manager.record_compression("test_backend", "1.0.0", stats);
        std::cout << "Statistics recorded successfully" << std::endl;
        
        // Test getting backend stats
        auto backend_stats = stats_manager.get_backend_stats("test_backend");
        std::cout << "Backend stats retrieved successfully" << std::endl;
        std::cout << "Backend name: " << backend_stats.backend_name << std::endl;
        std::cout << "Backend version: " << backend_stats.backend_version << std::endl;
        std::cout << "Total compressions: " << backend_stats.total_compressions.load() << std::endl;
        std::cout << "Successful compressions: " << backend_stats.successful_compressions.load() << std::endl;
        std::cout << "Success rate: " << backend_stats.success_rate() << "%" << std::endl;
        
        // Test global stats
        auto global_stats = stats_manager.get_global_stats();
        std::cout << "Global stats retrieved successfully" << std::endl;
        std::cout << "Global total compressions: " << global_stats.total_compressions.load() << std::endl;
        
        // Test export functionality
        std::string json_export = stats_manager.export_json();
        std::cout << "JSON export created successfully" << std::endl;
        std::cout << "JSON length: " << json_export.length() << " characters" << std::endl;
        
        std::string csv_export = stats_manager.export_csv();
        std::cout << "CSV export created successfully" << std::endl;
        std::cout << "CSV length: " << csv_export.length() << " characters" << std::endl;
        
        // Test reset functionality
        stats_manager.reset_all_stats();
        std::cout << "Statistics reset successfully" << std::endl;
        
        auto reset_stats = stats_manager.get_backend_stats("test_backend");
        std::cout << "After reset - Total compressions: " << reset_stats.total_compressions.load() << std::endl;
        
        std::cout << "\n✓ All minimal statistics tests passed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "✗ Error during testing: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
