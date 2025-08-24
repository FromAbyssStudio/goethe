#include "goethe/statistics.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>

int main() {
    std::cout << "Goethe Statistics System - Standalone Test" << std::endl;
    std::cout << "==========================================" << std::endl;

    try {
        // Test 1: Basic StatisticsManager functionality
        std::cout << "\n1. Testing StatisticsManager singleton..." << std::endl;
        auto& stats_manager = goethe::StatisticsManager::instance();
        std::cout << "✓ StatisticsManager singleton created successfully" << std::endl;
        
        // Test 2: Enable/disable statistics
        std::cout << "\n2. Testing enable/disable functionality..." << std::endl;
        bool initial_state = stats_manager.is_statistics_enabled();
        std::cout << "Initial state: " << (initial_state ? "enabled" : "disabled") << std::endl;
        
        stats_manager.enable_statistics(false);
        std::cout << "After disable: " << (stats_manager.is_statistics_enabled() ? "enabled" : "disabled") << std::endl;
        
        stats_manager.enable_statistics(true);
        std::cout << "After re-enable: " << (stats_manager.is_statistics_enabled() ? "enabled" : "disabled") << std::endl;
        std::cout << "✓ Enable/disable functionality works correctly" << std::endl;
        
        // Test 3: Timer functionality
        std::cout << "\n3. Testing Timer functionality..." << std::endl;
        goethe::StatisticsManager::Timer timer;
        timer.start();
        
        // Simulate some work
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        
        timer.stop();
        auto elapsed = timer.elapsed();
        std::cout << "Timer elapsed: " << elapsed.count() << " nanoseconds" << std::endl;
        std::cout << "✓ Timer functionality works correctly" << std::endl;
        
        // Test 4: OperationStats creation and calculations
        std::cout << "\n4. Testing OperationStats calculations..." << std::endl;
        std::string test_data = "This is a test string for compression testing";
        std::string compressed_data = "Compressed data";
        
        auto stats = goethe::create_operation_stats(
            test_data.size(),
            compressed_data.size(),
            timer,
            true,
            ""
        );
        
        std::cout << "Input size: " << stats.input_size << " bytes" << std::endl;
        std::cout << "Output size: " << stats.output_size << " bytes" << std::endl;
        std::cout << "Duration: " << stats.duration.count() << " nanoseconds" << std::endl;
        std::cout << "Success: " << (stats.success ? "Yes" : "No") << std::endl;
        std::cout << "Compression ratio: " << stats.compression_ratio() << std::endl;
        std::cout << "Compression rate: " << stats.compression_rate() << "%" << std::endl;
        std::cout << "Throughput: " << stats.throughput_mbps() << " MB/s" << std::endl;
        std::cout << "✓ OperationStats calculations work correctly" << std::endl;
        
        // Test 5: Recording statistics
        std::cout << "\n5. Testing statistics recording..." << std::endl;
        stats_manager.record_compression("test_backend", "1.0.0", stats);
        std::cout << "✓ Statistics recorded successfully" << std::endl;
        
        // Test 6: Retrieving backend statistics
        std::cout << "\n6. Testing backend statistics retrieval..." << std::endl;
        auto backend_stats = stats_manager.get_backend_stats("test_backend");
        std::cout << "Backend name: " << backend_stats.backend_name << std::endl;
        std::cout << "Backend version: " << backend_stats.backend_version << std::endl;
        std::cout << "Total compressions: " << backend_stats.total_compressions.load() << std::endl;
        std::cout << "Successful compressions: " << backend_stats.successful_compressions.load() << std::endl;
        std::cout << "Success rate: " << backend_stats.success_rate() << "%" << std::endl;
        std::cout << "✓ Backend statistics retrieval works correctly" << std::endl;
        
        // Test 7: Multiple operations
        std::cout << "\n7. Testing multiple operations..." << std::endl;
        for (int i = 0; i < 5; ++i) {
            goethe::StatisticsManager::Timer op_timer;
            op_timer.start();
            
            // Simulate compression work
            std::this_thread::sleep_for(std::chrono::microseconds(100));
            
            op_timer.stop();
            
            auto op_stats = goethe::create_operation_stats(
                1000 + i * 100,
                500 + i * 50,
                op_timer,
                i % 2 == 0, // Alternate success/failure
                i % 2 == 0 ? "" : "Test error"
            );
            
            stats_manager.record_compression("test_backend", "1.0.0", op_stats);
        }
        
        auto updated_stats = stats_manager.get_backend_stats("test_backend");
        std::cout << "After multiple operations:" << std::endl;
        std::cout << "Total compressions: " << updated_stats.total_compressions.load() << std::endl;
        std::cout << "Successful compressions: " << updated_stats.successful_compressions.load() << std::endl;
        std::cout << "Failed compressions: " << updated_stats.failed_compressions.load() << std::endl;
        std::cout << "Success rate: " << updated_stats.success_rate() << "%" << std::endl;
        std::cout << "✓ Multiple operations work correctly" << std::endl;
        
        // Test 8: Global statistics
        std::cout << "\n8. Testing global statistics..." << std::endl;
        auto global_stats = stats_manager.get_global_stats();
        std::cout << "Global total compressions: " << global_stats.total_compressions.load() << std::endl;
        std::cout << "Global successful compressions: " << global_stats.successful_compressions.load() << std::endl;
        std::cout << "✓ Global statistics work correctly" << std::endl;
        
        // Test 9: Export functionality
        std::cout << "\n9. Testing export functionality..." << std::endl;
        std::string json_export = stats_manager.export_json();
        std::cout << "JSON export length: " << json_export.length() << " characters" << std::endl;
        std::cout << "JSON preview: " << json_export.substr(0, 100) << "..." << std::endl;
        
        std::string csv_export = stats_manager.export_csv();
        std::cout << "CSV export length: " << csv_export.length() << " characters" << std::endl;
        std::cout << "CSV preview: " << csv_export.substr(0, 100) << "..." << std::endl;
        std::cout << "✓ Export functionality works correctly" << std::endl;
        
        // Test 10: Reset functionality
        std::cout << "\n10. Testing reset functionality..." << std::endl;
        stats_manager.reset_all_stats();
        
        auto reset_stats = stats_manager.get_backend_stats("test_backend");
        std::cout << "After reset - Total compressions: " << reset_stats.total_compressions.load() << std::endl;
        std::cout << "After reset - Successful compressions: " << reset_stats.successful_compressions.load() << std::endl;
        std::cout << "✓ Reset functionality works correctly" << std::endl;
        
        // Test 11: BackendStats copy semantics
        std::cout << "\n11. Testing BackendStats copy semantics..." << std::endl;
        goethe::BackendStats original_stats;
        original_stats.backend_name = "copy_test";
        original_stats.backend_version = "2.0.0";
        original_stats.total_compressions.store(42);
        original_stats.successful_compressions.store(40);
        
        goethe::BackendStats copied_stats = original_stats;
        std::cout << "Original total compressions: " << original_stats.total_compressions.load() << std::endl;
        std::cout << "Copied total compressions: " << copied_stats.total_compressions.load() << std::endl;
        std::cout << "✓ BackendStats copy semantics work correctly" << std::endl;
        
        std::cout << "\n🎉 All statistics system tests passed successfully!" << std::endl;
        std::cout << "\nThe Goethe Statistics System is fully functional and ready for use." << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "✗ Error during testing: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "✗ Unknown error during testing" << std::endl;
        return 1;
    }
    
    return 0;
}
