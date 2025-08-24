#!/bin/bash

# Goethe Dialog System Build Script
# This script builds the Goethe Dialog System with proper configuration

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Default values
BUILD_TYPE="RelWithDebInfo"
BUILD_DIR="build"
CLEAN_BUILD=false
INSTALL=false
INSTALL_PREFIX="/usr/local"
VERBOSE=false

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

# Function to show usage
show_usage() {
    cat << EOF
Usage: $0 [OPTIONS]

Build the Goethe Dialog System

OPTIONS:
    -h, --help              Show this help message
    -c, --clean             Clean build directory before building
    -d, --debug             Build in debug mode
    -r, --release           Build in release mode
    -i, --install           Install after building
    -p, --prefix PATH       Installation prefix (default: /usr/local)
    -v, --verbose           Verbose output
    -b, --build-dir DIR     Build directory (default: build)

EXAMPLES:
    $0                    # Build with default settings
    $0 -c -d             # Clean build in debug mode
    $0 -i -p /opt/goethe # Build and install to /opt/goethe
    $0 -v -r             # Verbose release build

EOF
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_usage
            exit 0
            ;;
        -c|--clean)
            CLEAN_BUILD=true
            shift
            ;;
        -d|--debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        -r|--release)
            BUILD_TYPE="Release"
            shift
            ;;
        -i|--install)
            INSTALL=true
            shift
            ;;
        -p|--prefix)
            INSTALL_PREFIX="$2"
            shift 2
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        -b|--build-dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        *)
            print_error "Unknown option: $1"
            show_usage
            exit 1
            ;;
    esac
done

# Check if we're in the right directory
if [[ ! -f "CMakeLists.txt" ]]; then
    print_error "CMakeLists.txt not found. Please run this script from the project root."
    exit 1
fi

print_status "Building Goethe Dialog System"
print_status "Build type: $BUILD_TYPE"
print_status "Build directory: $BUILD_DIR"

# Clean build directory if requested
if [[ "$CLEAN_BUILD" == true ]]; then
    print_status "Cleaning build directory..."
    rm -rf "$BUILD_DIR"
fi

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure with CMake
print_status "Configuring with CMake..."
CMAKE_ARGS=(
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
    -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX"
)

if [[ "$VERBOSE" == true ]]; then
    CMAKE_ARGS+=(-DCMAKE_VERBOSE_MAKEFILE=ON)
fi

cmake "${CMAKE_ARGS[@]}" ..

# Build
print_status "Building..."
if [[ "$VERBOSE" == true ]]; then
    make VERBOSE=1
else
    make -j$(nproc)
fi

print_success "Build completed successfully!"

# Install if requested
if [[ "$INSTALL" == true ]]; then
    print_status "Installing to $INSTALL_PREFIX..."
    if [[ "$VERBOSE" == true ]]; then
        make install VERBOSE=1
    else
        make install
    fi
    print_success "Installation completed successfully!"
fi

# Show build artifacts
print_status "Build artifacts:"
ls -la

print_success "Goethe Dialog System build completed!"
