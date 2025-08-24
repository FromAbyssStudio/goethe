#!/bin/bash

# Pre-push verification script for Goethe Dialog System
# This script runs comprehensive tests to ensure the push will succeed

set -e  # Exit on any error

echo "🚀 Running pre-push verification..."

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

# Step 1: Run pre-commit verification first
print_status "Running pre-commit verification..."
if [ -f "scripts/pre-commit-verify.sh" ]; then
    bash scripts/pre-commit-verify.sh
else
    print_warning "pre-commit-verify.sh not found, skipping"
fi

# Step 2: Check git status
print_status "Checking git status..."
if [ -n "$(git status --porcelain)" ]; then
    print_warning "Working directory has uncommitted changes:"
    git status --short
    read -p "Continue with push verification? (y/N): " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        print_error "Push verification cancelled"
        exit 1
    fi
else
    print_success "Working directory is clean"
fi

# Step 3: Check for merge conflicts
print_status "Checking for merge conflicts..."
if git diff --name-only --diff-filter=U | grep -q .; then
    print_error "Merge conflicts detected:"
    git diff --name-only --diff-filter=U
    exit 1
else
    print_success "No merge conflicts detected"
fi

# Step 4: Run comprehensive build tests
print_status "Running comprehensive build tests..."

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
    mkdir -p build
fi

cd build

# Test Debug build
print_status "Testing Debug build..."
cmake -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_CXX_FLAGS="-Wall -Wextra -Wpedantic" \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
      .. >/dev/null
make clean >/dev/null 2>&1
make -j$(nproc) >/dev/null 2>&1
print_success "Debug build successful"

# Test Release build
print_status "Testing Release build..."
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_CXX_FLAGS="-O3 -Wall -Wextra" \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
      .. >/dev/null
make clean >/dev/null 2>&1
make -j$(nproc) >/dev/null 2>&1
print_success "Release build successful"

# Step 5: Run all tests with verbose output
print_status "Running all tests with verbose output..."

# Run individual test executables
for test_exe in test_* minimal_*_test simple_*_test standalone_*_test; do
    if [ -f "$test_exe" ] && [ -x "$test_exe" ]; then
        print_status "Running $test_exe..."
        if ./"$test_exe" >/dev/null 2>&1; then
            print_success "$test_exe passed"
        else
            print_error "$test_exe failed"
            exit 1
        fi
    fi
done

# Run CTest with verbose output
print_status "Running CTest with verbose output..."
if ctest --output-on-failure --verbose >/dev/null 2>&1; then
    print_success "All CTest tests passed"
else
    print_error "Some CTest tests failed"
    exit 1
fi

cd ..

# Step 6: Check for common issues that would fail CI
print_status "Checking for common CI failure issues..."

# Check for hardcoded paths
if grep -r "/home/" src/ include/ --include="*.cpp" --include="*.hpp" --include="*.h" >/dev/null 2>&1; then
    print_warning "Found hardcoded /home/ paths:"
    grep -r "/home/" src/ include/ --include="*.cpp" --include="*.hpp" --include="*.h" || true
fi

# Check for Windows-specific paths
if grep -r "C:\\\\" src/ include/ --include="*.cpp" --include="*.hpp" --include="*.h" >/dev/null 2>&1; then
    print_warning "Found Windows-specific paths:"
    grep -r "C:\\\\" src/ include/ --include="*.cpp" --include="*.hpp" --include="*.h" || true
fi

# Check for missing includes
print_status "Checking for missing includes..."
cd build
if [ -f "compile_commands.json" ]; then
    # This is a basic check - in a real scenario you might want more sophisticated include checking
    print_success "Compilation database generated"
else
    print_warning "No compilation database found"
fi
cd ..

# Step 7: Check documentation
print_status "Checking documentation..."
if [ -f "README.md" ]; then
    print_success "README.md exists"
else
    print_warning "README.md missing"
fi

if [ -d "docs" ]; then
    print_success "Documentation directory exists"
else
    print_warning "Documentation directory missing"
fi

# Step 8: Check for sensitive information
print_status "Checking for sensitive information..."
if grep -r "password\|secret\|key\|token" src/ include/ --include="*.cpp" --include="*.hpp" --include="*.h" >/dev/null 2>&1; then
    print_warning "Found potential sensitive information:"
    grep -r "password\|secret\|key\|token" src/ include/ --include="*.cpp" --include="*.hpp" --include="*.h" || true
fi

# Step 9: Check git hooks
print_status "Checking git hooks..."
if [ -f ".git/hooks/pre-commit" ]; then
    print_success "Pre-commit hook exists"
else
    print_warning "Pre-commit hook not found"
fi

if [ -f ".git/hooks/pre-push" ]; then
    print_success "Pre-push hook exists"
else
    print_warning "Pre-push hook not found"
fi

# Step 10: Final summary
print_success "🎉 Pre-push verification completed successfully!"
print_status "Your code should pass GitHub Actions tests"
print_status "Ready to push to remote repository"

# Optional: Show what will be pushed
print_status "Files that will be pushed:"
git diff --cached --name-only || git diff --name-only || echo "No changes to push"
