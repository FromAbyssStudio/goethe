#include "goethe/manager.hpp"
#include "goethe/statistics.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <chrono>
#include <thread>
#include <random>

void print_usage(const char* program_name) {
    std::cout << "Usage: " << program_name << " <command> [options]\n\n";
    std::cout << "Commands:\n";
    std::cout << "  info                    - Show current backend information\n";
    std::cout << "  stats                   - Show current statistics\n";
    std::cout << "  global                  - Show global statistics\n";
    std::cout << "  enable                  - Enable statistics collection\n";
    std::cout << "  disable                 - Disable statistics collection\n";
    std::cout << "  reset                   - Reset all statistics\n";
    std::cout << "  export-json <file>      - Export statistics to JSON file\n";
    std::cout << "  export-csv <file>       - Export statistics to CSV file\n";
    std::cout << "  benchmark <size>        - Run compression benchmark with given size (bytes)\n";
    std::cout << "  stress-test <count>     - Run stress test with given number of operations\n";
    std::cout << "  switch <backend>        - Switch to specified backend (zstd, null)\n";
    std::cout << "  help                    - Show this help message\n\n";
    std::cout << "Examples:\n";
    std::cout << "  " << program_name << " info\n";
    std::cout << "  " << program_name << " stats\n";
    std::cout << "  " << program_name << " benchmark 1048576\n";
    std::cout << "  " << program_name << " export-json stats.json\n";
    std::cout << "  " << program_name << " stress-test 1000\n";
}

void print_backend_info(const goethe::CompressionManager& manager) {
    std::cout << "Backend Information:\n";
    std::cout << "===================\n";
    std::cout << "Name: " << manager.get_backend_name() << "\n";
    std::cout << "Version: " << manager.get_backend_version() << "\n";
    std::cout << "Initialized: " << (manager.is_initialized() ? "Yes" : "No") << "\n";
    std::cout << "Statistics Enabled: " << (manager.is_statistics_enabled() ? "Yes" : "No") << "\n";
}

void print_statistics(const goethe::BackendStats& stats, const std::string& title) {
    std::cout << "\n" << title << ":\n";
    std::cout << std::string(title.length() + 1, '=') << "\n";
    
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Backend: " << stats.backend_name << " v" << stats.backend_version << "\n\n";
    
    std::cout << "Operations:\n";
    std::cout << "  Total Compressions: " << stats.total_compressions.load() << "\n";
    std::cout << "  Successful Compressions: " << stats.successful_compressions.load() << "\n";
    std::cout << "  Failed Compressions: " << stats.failed_compressions.load() << "\n";
    std::cout << "  Total Decompressions: " << stats.total_decompressions.load() << "\n";
    std::cout << "  Successful Decompressions: " << stats.successful_decompressions.load() << "\n";
    std::cout << "  Failed Decompressions: " << stats.failed_decompressions.load() << "\n";
    std::cout << "  Success Rate: " << stats.success_rate() << "%\n\n";
    
    std::cout << "Data Sizes:\n";
    std::cout << "  Total Input: " << stats.total_input_size.load() << " bytes\n";
    std::cout << "  Total Output: " << stats.total_output_size.load() << " bytes\n";
    std::cout << "  Total Compressed: " << stats.total_compressed_size.load() << " bytes\n";
    std::cout << "  Total Decompressed: " << stats.total_decompressed_size.load() << " bytes\n\n";
    
    std::cout << "Performance Metrics:\n";
    std::cout << "  Average Compression Ratio: " << stats.average_compression_ratio() << "\n";
    std::cout << "  Average Compression Rate: " << stats.average_compression_rate() << "%\n";
    std::cout << "  Average Compression Throughput: " << stats.average_compression_throughput_mbps() << " MB/s\n";
    std::cout << "  Average Decompression Throughput: " << stats.average_decompression_throughput_mbps() << " MB/s\n";
}

std::vector<uint8_t> generate_test_data(size_t size) {
    std::vector<uint8_t> data(size);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    
    for (size_t i = 0; i < size; ++i) {
        data[i] = dis(gen) % 20; // Limited range for better compression
    }
    
    return data;
}

void run_benchmark(goethe::CompressionManager& manager, size_t data_size) {
    std::cout << "Running benchmark with " << data_size << " bytes of data...\n";
    
    auto data = generate_test_data(data_size);
    
    auto start = std::chrono::high_resolution_clock::now();
    auto compressed = manager.compress(data);
    auto comp_end = std::chrono::high_resolution_clock::now();
    
    auto decomp_start = std::chrono::high_resolution_clock::now();
    auto decompressed = manager.decompress(compressed);
    auto decomp_end = std::chrono::high_resolution_clock::now();
    
    auto comp_duration = std::chrono::duration_cast<std::chrono::microseconds>(comp_end - start);
    auto decomp_duration = std::chrono::duration_cast<std::chrono::microseconds>(decomp_end - decomp_start);
    
    double comp_ratio = static_cast<double>(compressed.size()) / data.size();
    double comp_rate = (1.0 - comp_ratio) * 100.0;
    double comp_throughput = (static_cast<double>(data.size()) / (1024.0 * 1024.0)) / 
                           (static_cast<double>(comp_duration.count()) / 1e6);
    double decomp_throughput = (static_cast<double>(decompressed.size()) / (1024.0 * 1024.0)) / 
                             (static_cast<double>(decomp_duration.count()) / 1e6);
    
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Results:\n";
    std::cout << "  Compression: " << comp_duration.count() << " μs, " << comp_throughput << " MB/s\n";
    std::cout << "  Decompression: " << decomp_duration.count() << " μs, " << decomp_throughput << " MB/s\n";
    std::cout << "  Compression rate: " << comp_rate << "%\n";
    std::cout << "  Data integrity: " << (data == decompressed ? "✓ OK" : "✗ FAILED") << "\n";
}

void run_stress_test(goethe::CompressionManager& manager, int count) {
    std::cout << "Running stress test with " << count << " operations...\n";
    
    std::vector<size_t> sizes = {1024, 10240, 102400, 1048576}; // 1KB, 10KB, 100KB, 1MB
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> size_dis(0, sizes.size() - 1);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < count; ++i) {
        size_t data_size = sizes[size_dis(gen)];
        auto data = generate_test_data(data_size);
        
        try {
            auto compressed = manager.compress(data);
            auto decompressed = manager.decompress(compressed);
            
            if (data != decompressed) {
                std::cout << "Data integrity check failed at operation " << i << "\n";
                return;
            }
            
            if ((i + 1) % 100 == 0) {
                std::cout << "Completed " << (i + 1) << " operations...\n";
            }
        } catch (const std::exception& e) {
            std::cout << "Error at operation " << i << ": " << e.what() << "\n";
            return;
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Stress test completed successfully!\n";
    std::cout << "Total time: " << duration.count() << " ms\n";
    std::cout << "Average time per operation: " << (duration.count() / static_cast<double>(count)) << " ms\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    std::string command = argv[1];
    
    try {
        auto& manager = goethe::CompressionManager::instance();
        manager.initialize(); // Auto-select best backend
        manager.enable_statistics(true);
        
        if (command == "help" || command == "--help" || command == "-h") {
            print_usage(argv[0]);
        } else if (command == "info") {
            print_backend_info(manager);
        } else if (command == "stats") {
            auto stats = manager.get_statistics();
            print_statistics(stats, "Current Backend Statistics");
        } else if (command == "global") {
            auto stats = manager.get_global_statistics();
            print_statistics(stats, "Global Statistics");
        } else if (command == "enable") {
            manager.enable_statistics(true);
            std::cout << "Statistics collection enabled.\n";
        } else if (command == "disable") {
            manager.enable_statistics(false);
            std::cout << "Statistics collection disabled.\n";
        } else if (command == "reset") {
            manager.reset_global_statistics();
            std::cout << "All statistics have been reset.\n";
        } else if (command == "export-json") {
            if (argc < 3) {
                std::cout << "Error: Please specify output file.\n";
                return 1;
            }
            std::string filename = argv[2];
            std::string json_data = manager.export_statistics_json();
            
            std::ofstream file(filename);
            if (file.is_open()) {
                file << json_data;
                file.close();
                std::cout << "Statistics exported to " << filename << "\n";
            } else {
                std::cout << "Error: Could not write to file " << filename << "\n";
                return 1;
            }
        } else if (command == "export-csv") {
            if (argc < 3) {
                std::cout << "Error: Please specify output file.\n";
                return 1;
            }
            std::string filename = argv[2];
            std::string csv_data = manager.export_statistics_csv();
            
            std::ofstream file(filename);
            if (file.is_open()) {
                file << csv_data;
                file.close();
                std::cout << "Statistics exported to " << filename << "\n";
            } else {
                std::cout << "Error: Could not write to file " << filename << "\n";
                return 1;
            }
        } else if (command == "benchmark") {
            if (argc < 3) {
                std::cout << "Error: Please specify data size in bytes.\n";
                return 1;
            }
            size_t data_size = std::stoul(argv[2]);
            run_benchmark(manager, data_size);
        } else if (command == "stress-test") {
            if (argc < 3) {
                std::cout << "Error: Please specify number of operations.\n";
                return 1;
            }
            int count = std::stoi(argv[2]);
            run_stress_test(manager, count);
        } else if (command == "switch") {
            if (argc < 3) {
                std::cout << "Error: Please specify backend name.\n";
                return 1;
            }
            std::string backend_name = argv[2];
            try {
                manager.switch_backend(backend_name);
                std::cout << "Switched to backend: " << manager.get_backend_name() << "\n";
            } catch (const std::exception& e) {
                std::cout << "Error switching backend: " << e.what() << "\n";
                return 1;
            }
        } else {
            std::cout << "Unknown command: " << command << "\n";
            print_usage(argv[0]);
            return 1;
        }
        
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}
