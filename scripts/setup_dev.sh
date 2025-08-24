#!/bin/bash

# Goethe Dialog System Development Setup Script
# This script sets up the development environment

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

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

# Detect OS
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    OS="linux"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    OS="macos"
elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "cygwin" ]]; then
    OS="windows"
else
    print_error "Unsupported operating system: $OSTYPE"
    exit 1
fi

print_status "Setting up Goethe Dialog System development environment on $OS"

# Check if we're in the right directory
if [[ ! -f "CMakeLists.txt" ]]; then
    print_error "CMakeLists.txt not found. Please run this script from the project root."
    exit 1
fi

# Install dependencies based on OS
install_dependencies() {
    case $OS in
        "linux")
            print_status "Installing dependencies on Linux..."
            
            # Detect package manager
            if command -v pacman &> /dev/null; then
                # Arch Linux
                print_status "Using pacman package manager"
                sudo pacman -S --needed cmake gcc clang yaml-cpp zstd openssl
            elif command -v apt &> /dev/null; then
                # Ubuntu/Debian
                print_status "Using apt package manager"
                sudo apt update
                sudo apt install -y cmake g++ clang libyaml-cpp-dev libzstd-dev libssl-dev
            elif command -v dnf &> /dev/null; then
                # Fedora
                print_status "Using dnf package manager"
                sudo dnf install -y cmake gcc-c++ clang yaml-cpp-devel libzstd-devel openssl-devel
            elif command -v yum &> /dev/null; then
                # CentOS/RHEL
                print_status "Using yum package manager"
                sudo yum install -y cmake gcc-c++ clang yaml-cpp-devel libzstd-devel openssl-devel
            else
                print_warning "Could not detect package manager. Please install dependencies manually:"
                echo "  - cmake (3.20+)"
                echo "  - gcc/g++ or clang"
                echo "  - yaml-cpp"
                echo "  - zstd (optional)"
                echo "  - openssl (optional)"
            fi
            ;;
        "macos")
            print_status "Installing dependencies on macOS..."
            if command -v brew &> /dev/null; then
                brew install cmake yaml-cpp zstd openssl
            else
                print_warning "Homebrew not found. Please install dependencies manually:"
                echo "  - cmake (3.20+)"
                echo "  - yaml-cpp"
                echo "  - zstd (optional)"
                echo "  - openssl (optional)"
            fi
            ;;
        "windows")
            print_status "Installing dependencies on Windows..."
            print_warning "Please install dependencies manually on Windows:"
            echo "  - Visual Studio with C++ support"
            echo "  - cmake (3.20+)"
            echo "  - vcpkg (for yaml-cpp, zstd, openssl)"
            ;;
    esac
}

# Create necessary directories
create_directories() {
    print_status "Creating project directories..."
    mkdir -p build src include third_party scripts
    mkdir -p src/engine/core src/engine/util src/tools src/tests
    mkdir -p include/goethe src/engine/core/compression src/engine/core/compression/implementations
    print_success "Directories created"
}

# Set up git hooks (if git is available)
setup_git_hooks() {
    if command -v git &> /dev/null; then
        print_status "Setting up git hooks..."
        mkdir -p .git/hooks
        
        # Pre-commit hook
        cat > .git/hooks/pre-commit << 'EOF'
#!/bin/bash
# Pre-commit hook to check code style and run tests

echo "Running pre-commit checks..."

# Check if build script exists and is executable
if [[ -f "scripts/build.sh" ]]; then
    echo "Building project..."
    ./scripts/build.sh -c -d
    if [[ $? -ne 0 ]]; then
        echo "Build failed! Commit aborted."
        exit 1
    fi
fi

echo "Pre-commit checks passed!"
EOF
        chmod +x .git/hooks/pre-commit
        print_success "Git hooks configured"
    else
        print_warning "Git not found, skipping git hooks setup"
    fi
}

# Create development configuration
create_dev_config() {
    print_status "Creating development configuration..."
    
    # Create .vscode directory and settings
    mkdir -p .vscode
    cat > .vscode/settings.json << 'EOF'
{
    "cmake.configureOnOpen": true,
    "cmake.buildDirectory": "${workspaceFolder}/build",
    "cmake.generator": "Unix Makefiles",
    "cmake.debugConfig": {
        "stopAtEntry": true
    },
    "files.associations": {
        "*.hpp": "cpp",
        "*.cpp": "cpp",
        "*.h": "c",
        "*.c": "c",
        "*.yaml": "yaml",
        "*.yml": "yaml"
    },
    "C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools"
}
EOF

    # Create .vscode/tasks.json
    cat > .vscode/tasks.json << 'EOF'
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "Build",
            "type": "shell",
            "command": "./scripts/build.sh",
            "group": "build",
            "presentation": {
                "echo": true,
                "reveal": "always",
                "focus": false,
                "panel": "shared"
            },
            "problemMatcher": ["$gcc"]
        },
        {
            "label": "Clean Build",
            "type": "shell",
            "command": "./scripts/build.sh",
            "args": ["-c"],
            "group": "build",
            "presentation": {
                "echo": true,
                "reveal": "always",
                "focus": false,
                "panel": "shared"
            },
            "problemMatcher": ["$gcc"]
        },
        {
            "label": "Debug Build",
            "type": "shell",
            "command": "./scripts/build.sh",
            "args": ["-d"],
            "group": "build",
            "presentation": {
                "echo": true,
                "reveal": "always",
                "focus": false,
                "panel": "shared"
            },
            "problemMatcher": ["$gcc"]
        }
    ]
}
EOF

    print_success "Development configuration created"
}

# Main setup process
main() {
    print_status "Starting development environment setup..."
    
    install_dependencies
    create_directories
    setup_git_hooks
    create_dev_config
    
    print_success "Development environment setup completed!"
    print_status "Next steps:"
    echo "  1. Run './scripts/build.sh' to build the project"
    echo "  2. Run './scripts/build.sh -d' for debug build"
    echo "  3. Check the README.md for more information"
    echo "  4. Open the project in your preferred IDE"
}

# Run main function
main
