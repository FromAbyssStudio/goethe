#!/bin/bash

# Build script for Goethe Dialog Godot Extension

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}Building Goethe Dialog Extension...${NC}"

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ]; then
    echo -e "${RED}Error: CMakeLists.txt not found. Please run this script from the addons/goethe_dialog directory.${NC}"
    exit 1
fi

# Check if we're in the right project structure
if [ ! -d "../../src/engine" ]; then
    echo -e "${RED}Error: Cannot find the main Goethe library source. Please ensure this plugin is in the correct location.${NC}"
    echo -e "${YELLOW}Expected path: ../../src/engine${NC}"
    echo -e "${YELLOW}Current directory: $(pwd)${NC}"
    ls -la ../../
    exit 1
fi

# Create build directory
mkdir -p build
cd build

# Configure with CMake
echo -e "${YELLOW}Configuring with CMake...${NC}"
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo

# Build
echo -e "${YELLOW}Building...${NC}"
make -j$(nproc)

echo -e "${GREEN}Build completed successfully!${NC}"
echo -e "${YELLOW}This builds a basic test executable for now.${NC}"
echo -e "${YELLOW}To build the full Godot extension, you need to:${NC}"
echo -e "${YELLOW}1. Install godot-cpp: git clone https://github.com/godotengine/godot-cpp.git ~/.local/share/godot/godot-cpp${NC}"
echo -e "${YELLOW}2. Build godot-cpp: cd ~/.local/share/godot/godot-cpp && scons platform=linux target=template_release${NC}"
echo -e "${YELLOW}3. Update CMakeLists.txt to include Godot bindings${NC}"
