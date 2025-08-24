#!/bin/bash

# Goethe Dialog System - Pre-commit Hook
# This script runs before each commit to ensure code quality

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
LINT_SCRIPT="$PROJECT_ROOT/scripts/lint.sh"

# Function to print colored output
print_status() {
    echo -e "${BLUE}[PRE-COMMIT]${NC} $1"
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

# Function to check if files are staged
check_staged_files() {
    local staged_files=()
    while IFS= read -r -d '' file; do
        if [[ "$file" == *.cpp ]] || [[ "$file" == *.hpp ]] || [[ "$file" == *.h ]] || [[ "$file" == *.c ]]; then
            staged_files+=("$file")
        fi
    done < <(git diff --cached --name-only -z)
    
    echo "${staged_files[@]}"
}

# Function to run linting on staged files
run_linting() {
    local staged_files=("$@")
    
    if [[ ${#staged_files[@]} -eq 0 ]]; then
        print_status "No C/C++ files staged for commit"
        return 0
    fi
    
    print_status "Running linting on ${#staged_files[@]} staged files..."
    
    # Run clang-format check
    print_status "Checking code formatting..."
    local format_errors=false
    
    for file in "${staged_files[@]}"; do
        if ! clang-format --dry-run --Werror "$file" >/dev/null 2>&1; then
            print_error "Formatting issues found in: $file"
            format_errors=true
        fi
    done
    
    if [[ "$format_errors" == true ]]; then
        print_error "Code formatting check failed!"
        print_warning "Run 'scripts/lint.sh --fix' to fix formatting issues"
        return 1
    fi
    
    print_success "Code formatting check passed"
    
    # Run clang-tidy if compile_commands.json exists
    if [[ -f "$PROJECT_ROOT/compile_commands.json" ]]; then
        print_status "Running static analysis..."
        local tidy_errors=false
        
        for file in "${staged_files[@]}"; do
            # Skip header files without corresponding .cpp files
            if [[ "$file" == *.hpp ]] || [[ "$file" == *.h ]]; then
                local cpp_file="${file%.*}.cpp"
                if [[ ! -f "$cpp_file" ]]; then
                    continue
                fi
            fi
            
            if ! clang-tidy "$file" >/dev/null 2>&1; then
                print_error "Static analysis issues found in: $file"
                tidy_errors=true
            fi
        done
        
        if [[ "$tidy_errors" == true ]]; then
            print_error "Static analysis check failed!"
            print_warning "Run 'scripts/lint.sh --fix' to apply fixes"
            return 1
        fi
        
        print_success "Static analysis check passed"
    else
        print_warning "compile_commands.json not found, skipping static analysis"
        print_warning "Run 'cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..' in build directory"
    fi
    
    return 0
}

# Function to check for TODO/FIXME comments
check_todos() {
    local staged_files=("$@")
    local todos_found=false
    
    print_status "Checking for TODO/FIXME comments..."
    
    for file in "${staged_files[@]}"; do
        if grep -q -i "TODO\|FIXME" "$file" 2>/dev/null; then
            print_warning "TODO/FIXME found in: $file"
            todos_found=true
        fi
    done
    
    if [[ "$todos_found" == true ]]; then
        print_warning "TODO/FIXME comments found in staged files"
        print_warning "Consider addressing these before committing"
    else
        print_success "No TODO/FIXME comments found"
    fi
}

# Function to check for debug code
check_debug_code() {
    local staged_files=("$@")
    local debug_found=false
    
    print_status "Checking for debug code..."
    
    for file in "${staged_files[@]}"; do
        if grep -q -E "(printf|cout|cerr|std::cout|std::cerr)" "$file" 2>/dev/null; then
            print_warning "Potential debug output found in: $file"
            debug_found=true
        fi
    done
    
    if [[ "$debug_found" == true ]]; then
        print_warning "Potential debug code found in staged files"
        print_warning "Consider removing debug output before committing"
    else
        print_success "No debug code found"
    fi
}

# Function to check file sizes
check_file_sizes() {
    local staged_files=("$@")
    local large_files=()
    
    print_status "Checking file sizes..."
    
    for file in "${staged_files[@]}"; do
        local size=$(wc -c < "$file" 2>/dev/null || echo 0)
        if [[ $size -gt 10000 ]]; then  # 10KB limit
            large_files+=("$file ($(($size / 1024))KB)")
        fi
    done
    
    if [[ ${#large_files[@]} -gt 0 ]]; then
        print_warning "Large files found:"
        for file in "${large_files[@]}"; do
            echo "  - $file"
        done
        print_warning "Consider splitting large files"
    else
        print_success "All files are reasonably sized"
    fi
}

# Main execution
main() {
    print_status "Running pre-commit checks..."
    echo
    
    # Get staged files
    local staged_files=()
    mapfile -t staged_files < <(check_staged_files)
    
    if [[ ${#staged_files[@]} -eq 0 ]]; then
        print_status "No C/C++ files staged, skipping checks"
        exit 0
    fi
    
    print_status "Found ${#staged_files[@]} staged C/C++ files"
    echo
    
    # Run all checks
    local exit_code=0
    
    # Linting checks (blocking)
    if ! run_linting "${staged_files[@]}"; then
        exit_code=1
    fi
    
    echo
    
    # Non-blocking checks
    check_todos "${staged_files[@]}"
    echo
    
    check_debug_code "${staged_files[@]}"
    echo
    
    check_file_sizes "${staged_files[@]}"
    echo
    
    # Final result
    if [[ $exit_code -eq 0 ]]; then
        print_success "All pre-commit checks passed!"
        print_success "Proceeding with commit..."
    else
        print_error "Pre-commit checks failed!"
        print_error "Please fix the issues above before committing"
        print_error "You can bypass this hook with: git commit --no-verify"
    fi
    
    exit $exit_code
}

# Run main function
main "$@"

