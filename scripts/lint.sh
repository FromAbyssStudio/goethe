#!/bin/bash

# Goethe Dialog System - Linting Script
# This script runs code formatting and static analysis

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SOURCE_DIRS=("src" "include")
FILE_EXTENSIONS=("cpp" "hpp" "h" "c")
FORMAT_ONLY=false
TIDY_ONLY=false
FIX=false
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

Options:
    -h, --help          Show this help message
    -f, --format-only   Only run clang-format (no clang-tidy)
    -t, --tidy-only     Only run clang-tidy (no clang-format)
    --fix               Apply fixes automatically (where possible)
    -v, --verbose       Verbose output
    --check             Check formatting without modifying files

Examples:
    $0                    # Run both format and tidy
    $0 --format-only      # Only format code
    $0 --tidy-only        # Only run static analysis
    $0 --fix              # Apply automatic fixes
    $0 --check            # Check formatting without changes

EOF
}

# Function to check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to find files to lint
find_source_files() {
    local files=()
    for dir in "${SOURCE_DIRS[@]}"; do
        if [[ -d "$PROJECT_ROOT/$dir" ]]; then
            for ext in "${FILE_EXTENSIONS[@]}"; do
                while IFS= read -r -d '' file; do
                    files+=("$file")
                done < <(find "$PROJECT_ROOT/$dir" -name "*.${ext}" -type f -print0)
            done
        fi
    done
    echo "${files[@]}"
}

# Function to run clang-format
run_clang_format() {
    local files=("$@")
    local format_args=()
    
    if [[ "$FIX" == true ]]; then
        format_args+=("-i")
        print_status "Running clang-format to fix formatting..."
    else
        format_args+=("--dry-run" "--Werror")
        print_status "Checking code formatting..."
    fi
    
    local has_errors=false
    
    for file in "${files[@]}"; do
        if [[ "$VERBOSE" == true ]]; then
            echo "  Processing: $file"
        fi
        
        if ! clang-format "${format_args[@]}" "$file" >/dev/null 2>&1; then
            print_error "Formatting issues found in: $file"
            has_errors=true
        fi
    done
    
    if [[ "$has_errors" == true ]]; then
        if [[ "$FIX" == true ]]; then
            print_success "Code formatting applied successfully"
        else
            print_error "Code formatting check failed. Run with --fix to apply fixes."
            return 1
        fi
    else
        print_success "Code formatting check passed"
    fi
}

# Function to run clang-tidy
run_clang_tidy() {
    local files=("$@")
    local tidy_args=()
    
    # Build compile_commands.json if it doesn't exist
    if [[ ! -f "$PROJECT_ROOT/compile_commands.json" ]]; then
        print_status "Building compile_commands.json..."
        cd "$PROJECT_ROOT"
        mkdir -p build
        cd build
        cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
        cd ..
        if [[ -f "build/compile_commands.json" ]]; then
            ln -sf build/compile_commands.json .
        fi
    fi
    
    if [[ "$FIX" == true ]]; then
        tidy_args+=("-fix")
        print_status "Running clang-tidy to apply fixes..."
    else
        print_status "Running clang-tidy static analysis..."
    fi
    
    local has_errors=false
    
    for file in "${files[@]}"; do
        if [[ "$VERBOSE" == true ]]; then
            echo "  Analyzing: $file"
        fi
        
        # Skip header files for clang-tidy if they don't have corresponding .cpp files
        if [[ "$file" == *.hpp ]] || [[ "$file" == *.h ]]; then
            local cpp_file="${file%.*}.cpp"
            if [[ ! -f "$cpp_file" ]]; then
                continue
            fi
        fi
        
        if ! clang-tidy "${tidy_args[@]}" "$file" >/dev/null 2>&1; then
            print_error "Static analysis issues found in: $file"
            has_errors=true
        fi
    done
    
    if [[ "$has_errors" == true ]]; then
        if [[ "$FIX" == true ]]; then
            print_success "Static analysis fixes applied successfully"
        else
            print_error "Static analysis check failed. Run with --fix to apply fixes."
            return 1
        fi
    else
        print_success "Static analysis check passed"
    fi
}

# Function to check dependencies
check_dependencies() {
    local missing_deps=()
    
    if ! command_exists clang-format; then
        missing_deps+=("clang-format")
    fi
    
    if ! command_exists clang-tidy; then
        missing_deps+=("clang-tidy")
    fi
    
    if [[ ${#missing_deps[@]} -gt 0 ]]; then
        print_error "Missing dependencies: ${missing_deps[*]}"
        echo "Please install the missing tools:"
        echo "  Ubuntu/Debian: sudo apt install clang-format clang-tidy"
        echo "  Arch Linux: sudo pacman -S clang"
        echo "  macOS: brew install llvm"
        exit 1
    fi
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_usage
            exit 0
            ;;
        -f|--format-only)
            FORMAT_ONLY=true
            shift
            ;;
        -t|--tidy-only)
            TIDY_ONLY=true
            shift
            ;;
        --fix)
            FIX=true
            shift
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        --check)
            FIX=false
            shift
            ;;
        *)
            print_error "Unknown option: $1"
            show_usage
            exit 1
            ;;
    esac
done

# Main execution
main() {
    print_status "Goethe Dialog System - Linting Script"
    echo
    
    # Check dependencies
    check_dependencies
    
    # Find source files
    print_status "Finding source files..."
    mapfile -t source_files < <(find_source_files)
    
    if [[ ${#source_files[@]} -eq 0 ]]; then
        print_warning "No source files found"
        exit 0
    fi
    
    print_status "Found ${#source_files[@]} source files"
    
    # Run linting
    local exit_code=0
    
    if [[ "$FORMAT_ONLY" == true ]]; then
        run_clang_format "${source_files[@]}" || exit_code=1
    elif [[ "$TIDY_ONLY" == true ]]; then
        run_clang_tidy "${source_files[@]}" || exit_code=1
    else
        run_clang_format "${source_files[@]}" || exit_code=1
        echo
        run_clang_tidy "${source_files[@]}" || exit_code=1
    fi
    
    echo
    if [[ $exit_code -eq 0 ]]; then
        print_success "All linting checks passed!"
    else
        print_error "Linting checks failed!"
    fi
    
    exit $exit_code
}

# Run main function
main "$@"

