#include "goethe/backend.hpp"
#include "goethe/factory.hpp"
#include "goethe/manager.hpp"
#include "goethe/register_backends.hpp"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <string>
#include <vector>

class CompressionTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Register all available backends
        goethe::register_compression_backends();
    }
    
    void TearDown() override {
        // Common cleanup for all tests
    }
    
    // Test data
    std::string test_data = "This is a test string that will be compressed and decompressed to verify the compression system works correctly.";
    std::vector<uint8_t> test_binary_data = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A};
};

// Factory tests
class CompressionFactoryTest : public CompressionTest {
protected:
    goethe::CompressionFactory& factory = goethe::CompressionFactory::instance();
};

TEST_F(CompressionFactoryTest, CreateNullBackend) {
    auto backend = factory.create_backend("null");
    EXPECT_NE(backend, nullptr);
    
    // Test that it's actually a null backend
    std::vector<uint8_t> test_data_vec(test_data.begin(), test_data.end());
    auto compressed = backend->compress(test_data_vec);
    auto decompressed = backend->decompress(compressed);
    EXPECT_EQ(decompressed, test_data_vec);
}

#ifdef GOETHE_ZSTD_AVAILABLE
TEST_F(CompressionFactoryTest, CreateZstdBackend) {
    auto backend = factory.create_backend("zstd");
    EXPECT_NE(backend, nullptr);
    
    // Test that it's actually a zstd backend (should compress)
    std::vector<uint8_t> test_data_vec(test_data.begin(), test_data.end());
    auto compressed = backend->compress(test_data_vec);
    EXPECT_LT(compressed.size(), test_data_vec.size()); // Should compress
    
    auto decompressed = backend->decompress(compressed);
    EXPECT_EQ(decompressed, test_data_vec);
}
#endif

TEST_F(CompressionFactoryTest, CreateInvalidBackend) {
    EXPECT_THROW(factory.create_backend("invalid_backend"), goethe::CompressionError);
}

TEST_F(CompressionFactoryTest, CreateBackendCaseInsensitive) {
    // Test case sensitivity - should be case sensitive
    EXPECT_THROW(factory.create_backend("NULL"), goethe::CompressionError);
    EXPECT_THROW(factory.create_backend("Null"), goethe::CompressionError);
    
    auto backend = factory.create_backend("null");
    EXPECT_NE(backend, nullptr);
}

TEST_F(CompressionFactoryTest, GetAvailableBackends) {
    auto backends = factory.get_available_backends();
    EXPECT_FALSE(backends.empty());
    EXPECT_TRUE(std::find(backends.begin(), backends.end(), "null") != backends.end());
    
#ifdef GOETHE_ZSTD_AVAILABLE
    EXPECT_TRUE(std::find(backends.begin(), backends.end(), "zstd") != backends.end());
#endif
}

// Manager tests
class CompressionManagerTest : public CompressionTest {
protected:
    goethe::CompressionManager& manager = goethe::CompressionManager::instance();
};

TEST_F(CompressionManagerTest, ManagerCompressDecompress) {
    // Initialize manager
    manager.initialize("null");
    
    std::vector<uint8_t> original_data(test_data.begin(), test_data.end());
    
    // Compress using manager
    auto compressed = manager.compress(original_data);
    EXPECT_FALSE(compressed.empty());
    
    // Decompress using manager
    auto decompressed = manager.decompress(compressed);
    EXPECT_EQ(decompressed, original_data);
}

TEST_F(CompressionManagerTest, ManagerSetBackend) {
    // Set to null backend
    manager.switch_backend("null");
    
    std::vector<uint8_t> original_data(test_data.begin(), test_data.end());
    auto compressed = manager.compress(original_data);
    auto decompressed = manager.decompress(compressed);
    EXPECT_EQ(decompressed, original_data);
}

#ifdef GOETHE_ZSTD_AVAILABLE
TEST_F(CompressionManagerTest, ManagerSetZstdBackend) {
    // Set to zstd backend
    manager.switch_backend("zstd");
    
    std::vector<uint8_t> original_data(test_data.begin(), test_data.end());
    auto compressed = manager.compress(original_data);
    EXPECT_LT(compressed.size(), original_data.size()); // Should compress
    
    auto decompressed = manager.decompress(compressed);
    EXPECT_EQ(decompressed, original_data);
}
#endif

TEST_F(CompressionManagerTest, ManagerSetInvalidBackend) {
    // This should not throw but should keep the current backend
    EXPECT_NO_THROW(manager.switch_backend("invalid_backend"));
}

TEST_F(CompressionManagerTest, ManagerGetBackendName) {
    manager.switch_backend("null");
    auto backend_name = manager.get_backend_name();
    EXPECT_EQ(backend_name, "null");
    
#ifdef GOETHE_ZSTD_AVAILABLE
    manager.switch_backend("zstd");
    backend_name = manager.get_backend_name();
    EXPECT_EQ(backend_name, "zstd");
#endif
}

TEST_F(CompressionManagerTest, ManagerIsInitialized) {
    manager.initialize("null");
    EXPECT_TRUE(manager.is_initialized());
}

// Convenience function tests
TEST_F(CompressionTest, ConvenienceFunctions) {
    std::vector<uint8_t> original_data(test_data.begin(), test_data.end());
    
    // Test convenience functions
    auto compressed = goethe::compress_data(original_data.data(), original_data.size(), "null");
    EXPECT_FALSE(compressed.empty());
    
    auto decompressed = goethe::decompress_data(compressed.data(), compressed.size(), "null");
    EXPECT_EQ(decompressed, original_data);
}

// Error handling tests
TEST_F(CompressionTest, DecompressInvalidData) {
    auto backend = goethe::CompressionFactory::instance().create_backend("null");
    std::vector<uint8_t> invalid_data = {0xFF, 0xFF, 0xFF, 0xFF};
    
    EXPECT_THROW(backend->decompress(invalid_data), std::exception);
}

#ifdef GOETHE_ZSTD_AVAILABLE
TEST_F(CompressionTest, ZstdDecompressInvalidData) {
    auto backend = goethe::CompressionFactory::instance().create_backend("zstd");
    std::vector<uint8_t> invalid_data = {0xFF, 0xFF, 0xFF, 0xFF};
    
    EXPECT_THROW(backend->decompress(invalid_data), std::exception);
}
#endif

// Performance tests (basic)
TEST_F(CompressionTest, NullBackendPerformance) {
    auto backend = goethe::CompressionFactory::instance().create_backend("null");
    
    // Create larger dataset for performance testing
    std::string large_data;
    for (int i = 0; i < 10000; ++i) {
        large_data += "Performance test data. ";
    }
    std::vector<uint8_t> original_data(large_data.begin(), large_data.end());
    
    // Time compression
    auto start = std::chrono::high_resolution_clock::now();
    auto compressed = backend->compress(original_data);
    auto compress_end = std::chrono::high_resolution_clock::now();
    
    // Time decompression
    auto decompressed = backend->decompress(compressed);
    auto decompress_end = std::chrono::high_resolution_clock::now();
    
    auto compress_time = std::chrono::duration_cast<std::chrono::microseconds>(compress_end - start);
    auto decompress_time = std::chrono::duration_cast<std::chrono::microseconds>(decompress_end - compress_end);
    
    EXPECT_EQ(decompressed, original_data);
    EXPECT_LT(compress_time.count(), 1000000); // Should complete in less than 1 second
    EXPECT_LT(decompress_time.count(), 1000000); // Should complete in less than 1 second
}

#ifdef GOETHE_ZSTD_AVAILABLE
TEST_F(CompressionTest, ZstdBackendPerformance) {
    auto backend = goethe::CompressionFactory::instance().create_backend("zstd");
    
    // Create larger dataset for performance testing
    std::string large_data;
    for (int i = 0; i < 10000; ++i) {
        large_data += "Performance test data. ";
    }
    std::vector<uint8_t> original_data(large_data.begin(), large_data.end());
    
    // Time compression
    auto start = std::chrono::high_resolution_clock::now();
    auto compressed = backend->compress(original_data);
    auto compress_end = std::chrono::high_resolution_clock::now();
    
    // Time decompression
    auto decompressed = backend->decompress(compressed);
    auto decompress_end = std::chrono::high_resolution_clock::now();
    
    auto compress_time = std::chrono::duration_cast<std::chrono::microseconds>(compress_end - start);
    auto decompress_time = std::chrono::duration_cast<std::chrono::microseconds>(decompress_end - compress_end);
    
    EXPECT_EQ(decompressed, original_data);
    EXPECT_LT(compressed.size(), original_data.size()); // Should compress
    EXPECT_LT(compress_time.count(), 1000000); // Should complete in less than 1 second
    EXPECT_LT(decompress_time.count(), 1000000); // Should complete in less than 1 second
}
#endif

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
