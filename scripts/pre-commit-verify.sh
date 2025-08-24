#!/bin/bash

# Pre-commit verification script for Goethe Dialog System
# This script runs local tests to ensure GitHub Actions will pass

set -e  # Exit on any error

echo "🔍 Running pre-commit verification..."

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

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ]; then
    print_error "This script must be run from the project root directory"
    exit 1
fi

# Create build directory if it doesn't exist
if [ ! -d "buil" ]; then
    print_status "Creating build directory..."
    mkdir -p buil
fi

# Step 1: Check code formatting
print_status "Checking code formatting..."
if command -v clang-format >/dev/null 2>&1; then
    # Check if any files need formatting
    if find src include -name "*.cpp" -o -name "*.hpp" -o -name "*.h" | xargs clang-format --dry-run --Werror >/dev/null 2>&1; then
        print_success "Code formatting is correct"
    else
        print_error "Code formatting issues found. Run: find src include -name '*.cpp' -o -name '*.hpp' -o -name '*.h' | xargs clang-format -i"
        exit 1
    fi
else
    print_warning "clang-format not found, skipping format check"
fi

# Step 2: Configure and build
print_status "Configuring CMake..."
cd buil
cmake -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_CXX_FLAGS="-Wall -Wextra -Wpedantic" \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
      .. >/dev/null

print_status "Building project..."
make -j$(nproc) >/dev/null 2>&1

# Step 3: Run tests
print_status "Running tests..."

# Run dialog tests
if [ -f "test_dialog" ]; then
    print_status "Running dialog tests..."
    ./test_dialog >/dev/null 2>&1
    print_success "Dialog tests passed"
else
    print_warning "test_dialog executable not found"
fi

# Run compression tests
if [ -f "minimal_compression_test" ]; then
    print_status "Running compression tests..."
    ./minimal_compression_test >/dev/null 2>&1
    print_success "Compression tests passed"
else
    print_warning "minimal_compression_test executable not found"
fi

# Run statistics tests
if [ -f "simple_statistics_test" ]; then
    print_status "Running statistics tests..."
    ./simple_statistics_test >/dev/null 2>&1
    print_success "Statistics tests passed"
else
    print_warning "simple_statistics_test executable not found"
fi

# Run CTest
print_status "Running CTest..."
if ctest --output-on-failure --verbose >/dev/null 2>&1; then
    print_success "CTest passed"
else
    print_error "CTest failed"
    exit 1
fi

cd ..

# Step 4: Check for TODO/FIXME comments
print_status "Checking for TODO/FIXME comments..."
if grep -r "TODO\|FIXME" src/ include/ --include="*.cpp" --include="*.hpp" --include="*.h" >/dev/null 2>&1; then
    print_warning "Found TODO/FIXME comments in source files"
    grep -r "TODO\|FIXME" src/ include/ --include="*.cpp" --include="*.hpp" --include="*.h" || true
else
    print_success "No TODO/FIXME comments found"
fi

# Step 5: Check YAML syntax in workflows
print_status "Checking GitHub Actions workflow syntax..."
if command -v yamllint >/dev/null 2>&1; then
    if yamllint .github/workflows/*.yml >/dev/null 2>&1; then
        print_success "GitHub Actions workflows are valid"
    else
        print_error "GitHub Actions workflow syntax errors found"
        yamllint .github/workflows/*.yml
        exit 1
    fi
else
    print_warning "yamllint not found, skipping workflow syntax check"
fi

# Step 6: Check for deprecated GitHub Actions
print_status "Checking for deprecated GitHub Actions..."
if grep -r "actions/upload-artifact@v3" .github/workflows/ >/dev/null 2>&1; then
    print_error "Found deprecated actions/upload-artifact@v3 in workflows"
    grep -r "actions/upload-artifact@v3" .github/workflows/
    exit 1
else
    print_success "No deprecated GitHub Actions found"
fi

print_success "🎉 Pre-commit verification completed successfully!"
print_status "Your changes should pass GitHub Actions tests"
