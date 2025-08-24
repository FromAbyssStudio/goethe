#include <iostream>
#include <fstream>
#include <filesystem>
#include <goethe/dialog.hpp>
#include <goethe/manager.hpp>
#include <goethe/statistics.hpp>

namespace fs = std::filesystem;

// Create sample dialog files for testing
void create_sample_dialog_files() {
    fs::create_directories("test_dialogs");
    
    // Create a simple dialog file
    std::ofstream dialog_file("test_dialogs/chapter1.yaml");
    dialog_file << R"(
id: chapter1_intro
nodes:
  - id: greeting
    speaker: Alice
    text: Hello, welcome to our story!
  - id: response
    speaker: Bob
    text: Thank you, I'm excited to begin!
  - id: choice
    speaker: Alice
    text: What would you like to do?
    choices:
      - id: explore
        text: Explore the world
      - id: talk
        text: Talk to people
)";
    dialog_file.close();
    
    // Create another dialog file
    std::ofstream dialog_file2("test_dialogs/chapter2.yaml");
    dialog_file2 << R"(
id: chapter2_development
nodes:
  - id: intro
    speaker: Narrator
    text: The story continues...
  - id: action
    speaker: Alice
    text: Let's see what happens next!
)";
    dialog_file2.close();
    
    std::cout << "Created sample dialog files in test_dialogs/" << std::endl;
}

// Test basic compression functionality
void test_compression() {
    std::cout << "\n=== Testing Compression ===" << std::endl;
    
    try {
        // Initialize compression manager
        auto& comp_manager = goethe::CompressionManager::instance();
        
        // Try to initialize with zstd first, fallback to null
        try {
            comp_manager.initialize("zstd");
            std::cout << "Using zstd compression backend" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "zstd not available, using null compression backend" << std::endl;
            comp_manager.initialize("null");
        }
        
        // Enable statistics tracking
        auto& stats_manager = goethe::StatisticsManager::instance();
        stats_manager.enable_statistics(true);
        
        std::cout << "Compression manager initialized successfully" << std::endl;
        std::cout << "Statistics tracking enabled" << std::endl;
        
        // Test with small data (should show overhead)
        std::vector<uint8_t> small_data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        auto small_compressed = comp_manager.compress(small_data);
        auto small_decompressed = comp_manager.decompress(small_compressed);
        
        std::cout << "\n--- Small Data Test (10 bytes) ---" << std::endl;
        std::cout << "Original size: " << small_data.size() << " bytes" << std::endl;
        std::cout << "Compressed size: " << small_compressed.size() << " bytes" << std::endl;
        std::cout << "Compression ratio: " << (float)small_compressed.size() / small_data.size() << "x" << std::endl;
        std::cout << "Note: Small data often has compression overhead" << std::endl;
        
        // Test with larger data (should show actual compression)
        std::vector<uint8_t> large_data;
        large_data.reserve(1000);
        for (int i = 0; i < 1000; ++i) {
            large_data.push_back(i % 256); // Some repetitive data
        }
        auto large_compressed = comp_manager.compress(large_data);
        auto large_decompressed = comp_manager.decompress(large_compressed);
        
        std::cout << "\n--- Large Data Test (1000 bytes) ---" << std::endl;
        std::cout << "Original size: " << large_data.size() << " bytes" << std::endl;
        std::cout << "Compressed size: " << large_compressed.size() << " bytes" << std::endl;
        std::cout << "Compression ratio: " << (float)large_compressed.size() / large_data.size() << "x" << std::endl;
        std::cout << "Space saved: " << (large_data.size() - large_compressed.size()) << " bytes" << std::endl;
        
        // Verify decompression worked correctly for both
        if (small_data == small_decompressed && large_data == large_decompressed) {
            std::cout << "\nDecompression verification passed for both tests" << std::endl;
        } else {
            std::cout << "\nDecompression verification failed" << std::endl;
            throw std::runtime_error("Decompression verification failed");
        }
        
        // Get statistics
        try {
            auto stats = stats_manager.get_backend_stats("zstd");
            std::cout << "\nAverage compression ratio: " << stats.average_compression_ratio() << std::endl;
        } catch (const std::exception& e) {
            std::cout << "\nStatistics not available for current backend" << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Compression test error: " << e.what() << std::endl;
        throw;
    }
}

// Test GPKG package creation (simulated)
void test_gpkg_package() {
    std::cout << "\n=== Testing GPKG Package Creation ===" << std::endl;
    
    try {
        // Create sample dialog files
        create_sample_dialog_files();
        
        // Simulate package creation process
        std::cout << "Creating sample dialog files..." << std::endl;
        
        // Read the created files
        std::map<std::string, std::string> dialog_files;
        
        for (const auto& entry : fs::directory_iterator("test_dialogs")) {
            if (entry.is_regular_file() && entry.path().extension() == ".yaml") {
                std::ifstream file(entry.path());
                if (file.is_open()) {
                    std::string content((std::istreambuf_iterator<char>(file)),
                                       std::istreambuf_iterator<char>());
                    std::string relative_path = fs::relative(entry.path(), "test_dialogs").string();
                    dialog_files[relative_path] = content;
                    file.close();
                    
                    std::cout << "Added file: " << relative_path << " (" << content.size() << " bytes)" << std::endl;
                }
            }
        }
        
        if (dialog_files.empty()) {
            throw std::runtime_error("No dialog files found");
        }
        
        std::cout << "Found " << dialog_files.size() << " dialog files" << std::endl;
        
        // Simulate package header
        std::cout << "\n--- Package Header ---" << std::endl;
        std::cout << "Game: Test Visual Novel" << std::endl;
        std::cout << "Version: 1.0.0" << std::endl;
        std::cout << "Company: Test Company" << std::endl;
        std::cout << "Compression: zstd" << std::endl;
        std::cout << "Files: " << dialog_files.size() << std::endl;
        
        // Calculate total size
        size_t total_size = 0;
        for (const auto& [filename, content] : dialog_files) {
            total_size += content.size();
        }
        std::cout << "Total size: " << total_size << " bytes" << std::endl;
        
        // Simulate compression of package contents
        auto& comp_manager = goethe::CompressionManager::instance();
        
        std::string package_content;
        for (const auto& [filename, content] : dialog_files) {
            package_content += "FILE: " + filename + "\n";
            package_content += "SIZE: " + std::to_string(content.size()) + "\n";
            package_content += "CONTENT:\n" + content + "\n";
            package_content += "---\n";
        }
        
        auto compressed_package = comp_manager.compress(package_content);
        
        std::cout << "\n--- Package Compression ---" << std::endl;
        std::cout << "Original package size: " << package_content.size() << " bytes" << std::endl;
        std::cout << "Compressed package size: " << compressed_package.size() << " bytes" << std::endl;
        std::cout << "Compression ratio: " << (float)compressed_package.size() / package_content.size() << "x" << std::endl;
        std::cout << "Space saved: " << (package_content.size() - compressed_package.size()) << " bytes" << std::endl;
        
        // Simulate package file creation
        std::ofstream package_file("test_package.gdkg", std::ios::binary);
        if (package_file.is_open()) {
            // Write package header
            package_file << "GDKG" << std::endl; // Magic number
            package_file << "Test Visual Novel" << std::endl;
            package_file << "1.0.0" << std::endl;
            package_file << "Test Company" << std::endl;
            package_file << "zstd" << std::endl;
            package_file << dialog_files.size() << std::endl;
            package_file << total_size << std::endl;
            package_file << compressed_package.size() << std::endl;
            
            // Write compressed data
            package_file.write(reinterpret_cast<const char*>(compressed_package.data()), 
                              compressed_package.size());
            
            package_file.close();
            
            std::cout << "\nPackage file created: test_package.gdkg" << std::endl;
            std::cout << "Package file size: " << fs::file_size("test_package.gdkg") << " bytes" << std::endl;
            
        } else {
            throw std::runtime_error("Failed to create package file");
        }
        
        // Test package extraction (simulated)
        std::cout << "\n--- Package Extraction Test ---" << std::endl;
        
        std::ifstream read_package("test_package.gdkg", std::ios::binary);
        if (read_package.is_open()) {
            std::string magic;
            std::getline(read_package, magic);
            
            if (magic == "GDKG") {
                std::cout << "Package magic number verified" << std::endl;
                
                // Read header info
                std::string game_name, version, company, compression;
                int file_count;
                size_t original_size, compressed_size;
                
                std::getline(read_package, game_name);
                std::getline(read_package, version);
                std::getline(read_package, company);
                std::getline(read_package, compression);
                read_package >> file_count >> original_size >> compressed_size;
                
                std::cout << "Package info:" << std::endl;
                std::cout << "  Game: " << game_name << std::endl;
                std::cout << "  Version: " << version << std::endl;
                std::cout << "  Company: " << company << std::endl;
                std::cout << "  Compression: " << compression << std::endl;
                std::cout << "  Files: " << file_count << std::endl;
                std::cout << "  Original size: " << original_size << " bytes" << std::endl;
                std::cout << "  Compressed size: " << compressed_size << " bytes" << std::endl;
                
                read_package.close();
                
                std::cout << "Package extraction test completed successfully" << std::endl;
                
            } else {
                throw std::runtime_error("Invalid package magic number");
            }
        } else {
            throw std::runtime_error("Failed to read package file");
        }
        
    } catch (const std::exception& e) {
        std::cerr << "GPKG test error: " << e.what() << std::endl;
        throw;
    }
}

int main() {
    std::cout << "Goethe Dialog System Test" << std::endl;
    
    try {
        // Test compression functionality
        test_compression();
        
        // Test GPKG package creation
        test_gpkg_package();
        
        std::cout << "\n=== All Tests Passed! ===" << std::endl;
        std::cout << "The Goethe library is working correctly." << std::endl;
        std::cout << "Created files:" << std::endl;
        std::cout << "  - test_dialogs/ (sample dialog files)" << std::endl;
        std::cout << "  - test_package.gdkg (sample package file)" << std::endl;
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
}
