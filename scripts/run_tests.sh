#!/bin/bash

# Goethe Dialog System Test Runner
# This script builds and runs the test suite

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if we're in the project root
if [ ! -f "CMakeLists.txt" ]; then
    print_error "This script must be run from the project root directory"
    exit 1
fi

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
    print_status "Creating build directory..."
    mkdir -p build
fi

# Navigate to build directory
cd build

# Configure with CMake
print_status "Configuring project with CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Build the project
print_status "Building project..."
make -j$(nproc)

# Check if Google Test is available
if [ -f "test_dialog" ] && [ -f "test_compression" ]; then
    print_success "Google Test executables built successfully"
    
    # Run tests with CTest
    print_status "Running tests with CTest..."
    ctest --output-on-failure --verbose
    
    # Also run individual test executables for more detailed output
    print_status "Running dialog tests..."
    ./test_dialog --gtest_color=yes
    
    print_status "Running compression tests..."
    ./test_compression --gtest_color=yes
    
elif [ -f "simple_test" ]; then
    print_warning "Google Test not available, running simple test instead"
    print_status "Running simple test..."
    ./simple_test
else
    print_error "No test executables found"
    exit 1
fi

print_success "Test run completed!"

# Return to project root
cd ..

print_status "Test results:"
echo "  - Build directory: build/"
echo "  - Test executables: build/test_dialog, build/test_compression"
echo "  - To run tests manually: cd build && ./test_dialog"
echo "  - To run with CTest: cd build && ctest"
