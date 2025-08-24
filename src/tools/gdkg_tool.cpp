#include "../engine/core/package.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstring>
#include <iomanip>

namespace fs = std::filesystem;

void print_usage(const char* program_name) {
    std::cout << "Goethe Dialog Package Tool (gdkg)\n\n";
    std::cout << "Usage: " << program_name << " <command> [options]\n\n";
    std::cout << "Commands:\n";
    std::cout << "  create <output.gdkg> <input_directory> [options]  Create a new package\n";
    std::cout << "  extract <input.gdkg> <output_directory> [options] Extract package contents\n";
    std::cout << "  info <input.gdkg>                               Show package information\n";
    std::cout << "  list <input.gdkg>                               List package contents\n";
    std::cout << "  verify <input.gdkg> [options]                   Verify package integrity\n";
    std::cout << "  extract-file <input.gdkg> <filename> [options]   Extract specific file\n\n";
    std::cout << "Options:\n";
    std::cout << "  --game <name>           Set game name\n";
    std::cout << "  --version <version>     Set version\n";
    std::cout << "  --company <company>     Set company name\n";
    std::cout << "  --compression <backend> Set compression backend (zstd, null)\n";
    std::cout << "  --level <level>         Set compression level (1-22 for zstd)\n";
    std::cout << "  --encrypt <key>         Encrypt package with key\n";
    std::cout << "  --sign <key>            Sign package with key\n";
    std::cout << "  --decrypt <key>         Decrypt package with key\n";
    std::cout << "  --verify-signature <key> Verify package signature\n";
    std::cout << "  --no-encrypt            Disable encryption\n";
    std::cout << "  --no-sign               Disable signing\n";
    std::cout << "  --help                  Show this help message\n\n";
    std::cout << "Examples:\n";
    std::cout << "  " << program_name << " create game.gdkg ./dialog_files --game \"My Game\" --version \"1.0.0\" --company \"My Company\"\n";
    std::cout << "  " << program_name << " extract game.gdkg ./extracted --decrypt mykey\n";
    std::cout << "  " << program_name << " info game.gdkg\n";
    std::cout << "  " << program_name << " verify game.gdkg --verify-signature mykey\n";
}

bool read_yaml_files(const std::string& directory, std::map<std::string, std::string>& files) {
    try {
        for (const auto& entry : fs::recursive_directory_iterator(directory)) {
            if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();
                if (ext == ".yaml" || ext == ".yml") {
                    std::ifstream file(entry.path());
                    if (file.is_open()) {
                        std::string content((std::istreambuf_iterator<char>(file)),
                                           std::istreambuf_iterator<char>());
                        std::string relative_path = fs::relative(entry.path(), directory).string();
                        files[relative_path] = content;
                        file.close();
                    }
                }
            }
        }
        return !files.empty();
    } catch (const std::exception& e) {
        std::cerr << "Error reading directory: " << e.what() << std::endl;
        return false;
    }
}

int create_package(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Error: create command requires output file and input directory\n";
        return 1;
    }

    std::string output_file = argv[2];
    std::string input_directory = argv[3];

    // Parse options
    goethe::PackageOptions options;
    goethe::PackageHeader header;
    
    for (int i = 4; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--game" && i + 1 < argc) {
            header.game_name = argv[++i];
        } else if (arg == "--version" && i + 1 < argc) {
            header.version = argv[++i];
        } else if (arg == "--company" && i + 1 < argc) {
            header.company = argv[++i];
        } else if (arg == "--compression" && i + 1 < argc) {
            options.compression_backend = argv[++i];
        } else if (arg == "--level" && i + 1 < argc) {
            options.compression_level = std::stoi(argv[++i]);
        } else if (arg == "--encrypt" && i + 1 < argc) {
            options.encryption_key = argv[++i];
        } else if (arg == "--sign" && i + 1 < argc) {
            options.signature_key = argv[++i];
        } else if (arg == "--no-encrypt") {
            options.encrypt_content = false;
        } else if (arg == "--no-sign") {
            options.sign_package = false;
        }
    }

    // Set defaults
    if (header.game_name.empty()) {
        header.game_name = fs::path(output_file).stem().string();
    }
    if (header.version.empty()) {
        header.version = "1.0.0";
    }
    if (header.company.empty()) {
        header.company = "Unknown";
    }

    // Read YAML files
    std::map<std::string, std::string> yaml_files;
    if (!read_yaml_files(input_directory, yaml_files)) {
        std::cerr << "Error: No YAML files found in directory or directory not accessible\n";
        return 1;
    }

    std::cout << "Found " << yaml_files.size() << " YAML files\n";

    // Create package
    auto& package_manager = goethe::PackageManager::instance();
    if (package_manager.create_package(output_file, yaml_files, header, options)) {
        std::cout << "Package created successfully: " << output_file << std::endl;
        return 0;
    } else {
        std::cerr << "Error: Failed to create package\n";
        return 1;
    }
}

int extract_package(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Error: extract command requires input file and output directory\n";
        return 1;
    }

    std::string input_file = argv[2];
    std::string output_directory = argv[3];
    std::string decryption_key;
    std::string signature_key;

    // Parse options
    for (int i = 4; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--decrypt" && i + 1 < argc) {
            decryption_key = argv[++i];
        } else if (arg == "--verify-signature" && i + 1 < argc) {
            signature_key = argv[++i];
        }
    }

    // Create output directory
    try {
        fs::create_directories(output_directory);
    } catch (const std::exception& e) {
        std::cerr << "Error creating output directory: " << e.what() << std::endl;
        return 1;
    }

    // Extract package
    auto& package_manager = goethe::PackageManager::instance();
    if (package_manager.extract_package(input_file, output_directory, decryption_key, signature_key)) {
        std::cout << "Package extracted successfully to: " << output_directory << std::endl;
        return 0;
    } else {
        std::cerr << "Error: Failed to extract package\n";
        return 1;
    }
}

int show_info(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Error: info command requires input file\n";
        return 1;
    }

    std::string input_file = argv[2];
    auto& package_manager = goethe::PackageManager::instance();
    
    auto header = package_manager.read_header(input_file);
    if (!header) {
        std::cerr << "Error: Cannot read package header\n";
        return 1;
    }

    std::cout << "Package Information:\n";
    std::cout << "  Game: " << header->game_name << std::endl;
    std::cout << "  Version: " << header->version << std::endl;
    std::cout << "  Company: " << header->company << std::endl;
    std::cout << "  Compression: " << header->compression_backend << std::endl;
    std::cout << "  Files: " << header->file_count << std::endl;
    std::cout << "  Original Size: " << header->total_size << " bytes\n";
    std::cout << "  Compressed Size: " << header->compressed_size << " bytes\n";
    std::cout << "  Compression Ratio: " << std::fixed << std::setprecision(1) 
              << (100.0 - (double)header->compressed_size / header->total_size * 100.0) << "%\n";
    
    if (!header->signature_hash.empty()) {
        std::cout << "  Signed: Yes\n";
        std::cout << "  Signature: " << header->signature_hash.substr(0, 16) << "...\n";
    } else {
        std::cout << "  Signed: No\n";
    }

    auto creation_time = std::chrono::system_clock::from_time_t(header->creation_timestamp);
    auto time_t = std::chrono::system_clock::to_time_t(creation_time);
    std::cout << "  Created: " << std::ctime(&time_t);

    return 0;
}

int list_contents(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Error: list command requires input file\n";
        return 1;
    }

    std::string input_file = argv[2];
    auto& package_manager = goethe::PackageManager::instance();
    
    auto contents = package_manager.list_package_contents(input_file);
    if (contents.empty()) {
        std::cerr << "Error: Cannot read package contents\n";
        return 1;
    }

    std::cout << "Package Contents (" << contents.size() << " files):\n";
    for (const auto& filename : contents) {
        std::cout << "  " << filename << std::endl;
    }

    return 0;
}

int verify_package(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Error: verify command requires input file\n";
        return 1;
    }

    std::string input_file = argv[2];
    std::string signature_key;

    // Parse options
    for (int i = 3; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--verify-signature" && i + 1 < argc) {
            signature_key = argv[++i];
        }
    }

    auto& package_manager = goethe::PackageManager::instance();
    auto verification = package_manager.verify_package(input_file, signature_key);

    std::cout << "Package Verification:\n";
    std::cout << "  Valid: " << (verification.is_valid ? "Yes" : "No") << std::endl;
    std::cout << "  Signature Valid: " << (verification.signature_valid ? "Yes" : "No") << std::endl;
    std::cout << "  Content Valid: " << (verification.content_valid ? "Yes" : "No") << std::endl;
    
    if (!verification.error_message.empty()) {
        std::cout << "  Error: " << verification.error_message << std::endl;
    }
    
    if (!verification.warnings.empty()) {
        std::cout << "  Warnings:\n";
        for (const auto& warning : verification.warnings) {
            std::cout << "    " << warning << std::endl;
        }
    }

    return verification.is_valid ? 0 : 1;
}

int extract_file(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Error: extract-file command requires input file and filename\n";
        return 1;
    }

    std::string input_file = argv[2];
    std::string filename = argv[3];
    std::string decryption_key;

    // Parse options
    for (int i = 4; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--decrypt" && i + 1 < argc) {
            decryption_key = argv[++i];
        }
    }

    auto& package_manager = goethe::PackageManager::instance();
    auto content = package_manager.extract_file(input_file, filename, decryption_key);
    
    if (content) {
        std::cout << *content;
        return 0;
    } else {
        std::cerr << "Error: Failed to extract file or file not found\n";
        return 1;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    std::string command = argv[1];

    if (command == "create") {
        return create_package(argc, argv);
    } else if (command == "extract") {
        return extract_package(argc, argv);
    } else if (command == "info") {
        return show_info(argc, argv);
    } else if (command == "list") {
        return list_contents(argc, argv);
    } else if (command == "verify") {
        return verify_package(argc, argv);
    } else if (command == "extract-file") {
        return extract_file(argc, argv);
    } else if (command == "--help" || command == "-h") {
        print_usage(argv[0]);
        return 0;
    } else {
        std::cerr << "Error: Unknown command '" << command << "'\n";
        print_usage(argv[0]);
        return 1;
    }
}
