#include "goethe/null.hpp"
#include <cstring>

namespace goethe {

std::vector<uint8_t> NullCompressionBackend::compress(const uint8_t* data, std::size_t size) {
    validate_input(data, size);

    if (size == 0) {
        return {};
    }

    // Simply copy the data without compression
    std::vector<uint8_t> result(size);
    std::memcpy(result.data(), data, size);
    return result;
}

std::vector<uint8_t> NullCompressionBackend::decompress(const uint8_t* data, std::size_t size) {
    validate_input(data, size);

    if (size == 0) {
        return {};
    }

    // For null backend, validate that the data looks reasonable
    // This is a simple validation - if all bytes are the same value, it might be invalid
    if (size > 1) {
        bool all_same = true;
        uint8_t first_byte = data[0];
        for (std::size_t i = 1; i < size; ++i) {
            if (data[i] != first_byte) {
                all_same = false;
                break;
            }
        }

        // If all bytes are the same and it's a suspicious value (like 0xFF), throw an exception
        if (all_same && (first_byte == 0xFF || first_byte == 0x00)) {
            throw CompressionError("Null backend detected potentially invalid data");
        }
    }

    // Simply copy the data without decompression
    std::vector<uint8_t> result(size);
    std::memcpy(result.data(), data, size);
    return result;
}

} // namespace goethe
