# CI/CD Configuration

This document describes the GitHub Actions CI/CD setup for the Goethe Dialog System.

## Overview

The project uses multiple GitHub Actions workflows to ensure code quality, test coverage, and cross-platform compatibility:

## Workflows

### 1. Full Test Suite (`full-test-suite.yml`)
**Purpose**: Comprehensive testing across all platforms and configurations

**Features**:
- Matrix builds with multiple compilers (GCC 12, Clang 15)
- Multiple build types (Debug, Release)
- Multiple compression backends (zstd, null)
- Code quality checks (clang-tidy, clang-format, cppcheck)
- Sanitizer testing (Address, Undefined, Memory)
- Coverage reporting with Codecov integration
- Performance testing and benchmarking
- Cross-platform testing (Ubuntu, macOS, Windows)

**When it runs**: On push to `main`/`develop` and pull requests

### 2. Quick Test (`quick-test.yml`)
**Purpose**: Fast feedback during development

**Features**:
- Minimal matrix (GCC 12, Clang 15)
- Debug and Release builds
- Basic test execution
- Focused on key test executables

**When it runs**: On push to `main`/`develop` and pull requests

### 3. Cached Build (`cached-build.yml`)
**Purpose**: Optimized builds with dependency caching

**Features**:
- ccache for faster incremental builds
- Dependency caching
- Optimized for CI performance

**When it runs**: On push to `main`/`develop` and pull requests

### 4. Statistics Tests (`statistics-test.yml`)
**Purpose**: Specialized testing for statistics functionality

**Features**:
- Focused on statistics-related tests
- Multiple compression backends
- Individual test executable execution
- Statistics tool testing

**When it runs**: On push to `main`/`develop` and pull requests

### 5. Compression Tests (`compression-test.yml`)
**Purpose**: Specialized testing for compression functionality

**Features**:
- Compression backend testing
- Performance testing with large datasets
- Different data type testing
- Compression ratio validation

**When it runs**: On push to `main`/`develop` and pull requests

### 6. C++ Tests (`cpp-tests.yml`)
**Purpose**: Legacy comprehensive testing (being replaced by full-test-suite.yml)

**Features**:
- Multiple compiler versions
- Cross-platform builds
- Code quality checks
- Sanitizer testing
- Coverage reporting

**When it runs**: On push to `main`/`develop` and pull requests

## Test Executables

The following test executables are built and run:

### Core Tests
- `simple_test` - Basic functionality tests
- `test_dialog` - Dialog system tests
- `test_compression` - Compression functionality tests
- `test_basic` - Basic Google Test framework tests

### Statistics Tests
- `simple_statistics_test` - Basic statistics functionality
- `statistics_test` - Comprehensive statistics testing
- `minimal_statistics_test` - Minimal statistics validation
- `standalone_statistics_test` - Standalone statistics tests

### Compression Tests
- `minimal_compression_test` - Basic compression validation

### Tools
- `statistics_tool` - Statistics analysis tool

## Build Configurations

### Compilers
- **GCC**: 11, 12, 13
- **Clang**: 14, 15, 16
- **MSVC**: Latest (Windows)
- **Apple Clang**: Latest (macOS)

### Build Types
- **Debug**: Full debugging information, assertions enabled
- **Release**: Optimized builds for production
- **RelWithDebInfo**: Release with debug information

### Compression Backends
- **zstd**: Zstandard compression (when available)
- **null**: No compression (fallback)

## Code Quality Checks

### Static Analysis
- **clang-tidy**: Modern C++ best practices
- **cppcheck**: Static code analysis
- **clang-format**: Code formatting consistency

### Sanitizers
- **AddressSanitizer**: Memory error detection
- **UndefinedBehaviorSanitizer**: Undefined behavior detection
- **MemorySanitizer**: Memory access validation

### Coverage
- **lcov**: Line coverage reporting
- **Codecov**: Coverage visualization and tracking

## Performance Testing

### Benchmarks
- Compression performance testing
- Statistics collection performance
- Large dataset processing
- Memory usage analysis

### Metrics
- Compression ratios
- Throughput measurements
- Memory consumption
- CPU utilization

## Artifacts

The following artifacts are generated and stored:

### Build Artifacts
- Compiled binaries
- Test results
- Coverage reports
- Static analysis results

### Test Results
- CTest output
- Individual test executable output
- Performance benchmarks
- Error logs

## Dependencies

### Required Packages
- **CMake**: 3.20+
- **yaml-cpp**: YAML parsing
- **Google Test**: Unit testing framework
- **OpenSSL**: Cryptography (optional)
- **zstd**: Compression (optional)
- **pkg-config**: Package configuration

### Platform-Specific
- **Ubuntu**: apt packages
- **macOS**: Homebrew packages
- **Windows**: vcpkg packages

## Local Development

### Running Tests Locally
```bash
# Build and test
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)
ctest --output-on-failure --verbose

# Run individual tests
./simple_statistics_test
./statistics_test
./minimal_compression_test
```

### Code Quality Checks
```bash
# Format code
find src include -name "*.cpp" -o -name "*.hpp" -o -name "*.h" | xargs clang-format -i

# Run clang-tidy
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_CLANG_TIDY=clang-tidy ..
make

# Run cppcheck
cppcheck --enable=all --std=c++20 src/ include/
```

## Troubleshooting

### Common Issues

1. **Missing Dependencies**
   - Ensure all required packages are installed
   - Check platform-specific installation instructions

2. **Test Failures**
   - Check test output for specific error messages
   - Verify test data and environment setup
   - Review recent code changes

3. **Build Failures**
   - Check compiler compatibility
   - Verify CMake configuration
   - Review dependency versions

4. **Performance Issues**
   - Check system resources
   - Review optimization flags
   - Analyze benchmark results

### Debugging

1. **Verbose Output**
   - Use `VERBOSE=1` with make
   - Enable detailed CTest output
   - Check individual test executable output

2. **Sanitizer Issues**
   - Run with AddressSanitizer for memory issues
   - Use UndefinedBehaviorSanitizer for UB detection
   - Check sanitizer output for specific errors

3. **Coverage Issues**
   - Verify coverage flags are set
   - Check lcov configuration
   - Review coverage exclusion patterns

## Best Practices

### For Developers
1. Run tests locally before pushing
2. Use appropriate build types for testing
3. Check code quality tools output
4. Monitor performance benchmarks
5. Review coverage reports

### For CI/CD
1. Use appropriate workflow for the task
2. Monitor build times and optimize
3. Review test results and artifacts
4. Address code quality issues promptly
5. Maintain cross-platform compatibility

## Future Improvements

### Planned Enhancements
1. **Parallel Testing**: Increase test parallelism
2. **Dependency Management**: Improve dependency handling
3. **Performance Monitoring**: Add performance regression detection
4. **Security Scanning**: Add security vulnerability scanning
5. **Automated Releases**: Add automated release workflows

### Optimization Opportunities
1. **Build Caching**: Improve ccache effectiveness
2. **Test Selection**: Implement smart test selection
3. **Resource Usage**: Optimize resource allocation
4. **Artifact Management**: Improve artifact storage and retrieval
