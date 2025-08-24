#!/bin/bash

# Local test script that mimics GitHub Actions workflow
# Run this to test your build before pushing to GitHub

set -e  # Exit on any error

echo "🧪 Running local C++ tests for Goethe Dialog System"
echo "=================================================="

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${GREEN}✅ $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠️  $1${NC}"
}

print_error() {
    echo -e "${RED}❌ $1${NC}"
}

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ]; then
    print_error "CMakeLists.txt not found. Please run this script from the project root."
    exit 1
fi

# Check dependencies
echo "📦 Checking dependencies..."

# Check for required packages
DEPS=("cmake" "make" "g++" "clang++")
MISSING_DEPS=()

for dep in "${DEPS[@]}"; do
    if ! command -v "$dep" &> /dev/null; then
        MISSING_DEPS+=("$dep")
    fi
done

if [ ${#MISSING_DEPS[@]} -ne 0 ]; then
    print_warning "Missing dependencies: ${MISSING_DEPS[*]}"
    echo "Install with: sudo pacman -S cmake make gcc clang (Arch Linux)"
    echo "Or: sudo apt-get install cmake build-essential clang (Ubuntu)"
fi

# Check for optional packages
OPTIONAL_DEPS=("yaml-cpp" "gtest" "openssl" "zstd")
for dep in "${OPTIONAL_DEPS[@]}"; do
    if ! pkg-config --exists "$dep" 2>/dev/null; then
        print_warning "Optional dependency not found: $dep"
    fi
done

print_status "Dependencies checked"

# Clean previous builds
echo "🧹 Cleaning previous builds..."
rm -rf build/
print_status "Build directory cleaned"

# Test with GCC
echo "🔨 Testing with GCC..."
mkdir -p build-gcc
cd build-gcc
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ ..
make -j$(nproc)
print_status "GCC build successful"

echo "🧪 Running GCC tests..."
ctest --output-on-failure --verbose
print_status "GCC tests passed"
cd ..

# Test with Clang (if available)
if command -v clang++ &> /dev/null; then
    echo "🔨 Testing with Clang..."
    mkdir -p build-clang
    cd build-clang
    cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ ..
    make -j$(nproc)
    print_status "Clang build successful"
    
    echo "🧪 Running Clang tests..."
    ctest --output-on-failure --verbose
    print_status "Clang tests passed"
    cd ..
else
    print_warning "Clang not found, skipping Clang tests"
fi

# Code formatting check (if clang-format is available)
if command -v clang-format &> /dev/null; then
    echo "🎨 Checking code formatting..."
    if find src include -name "*.cpp" -o -name "*.hpp" -o -name "*.h" | xargs clang-format --dry-run --Werror; then
        print_status "Code formatting check passed"
    else
        print_error "Code formatting check failed"
        echo "Run: find src include -name '*.cpp' -o -name '*.hpp' -o -name '*.h' | xargs clang-format -i"
        exit 1
    fi
else
    print_warning "clang-format not found, skipping formatting check"
fi

# Clang-tidy check (if available)
if command -v clang-tidy &> /dev/null; then
    echo "🔍 Running clang-tidy..."
    mkdir -p build-tidy
    cd build-tidy
    if cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_CLANG_TIDY=clang-tidy .. && make -j$(nproc); then
        print_status "Clang-tidy check passed"
    else
        print_error "Clang-tidy check failed"
        exit 1
    fi
    cd ..
else
    print_warning "clang-tidy not found, skipping static analysis"
fi

echo ""
echo "🎉 All tests completed successfully!"
echo "=================================="
print_status "Your code is ready for GitHub Actions"
echo ""
echo "Next steps:"
echo "1. Commit your changes"
echo "2. Push to GitHub"
echo "3. Check the Actions tab to see CI results"
echo ""
echo "To add status badges to your README, see .github/badges.yml"
