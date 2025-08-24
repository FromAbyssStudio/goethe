#!/bin/bash

# Install git hooks for Goethe Dialog System
# This script sets up pre-commit and pre-push hooks

set -e

echo "🔧 Installing git hooks..."

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Check if we're in a git repository
if [ ! -d ".git" ]; then
    echo "❌ Error: This script must be run from a git repository root"
    exit 1
fi

# Create hooks directory if it doesn't exist
mkdir -p .git/hooks

# Copy pre-commit hook
if [ -f "scripts/pre-commit-verify.sh" ]; then
    cp .git/hooks/pre-commit .git/hooks/pre-commit.backup 2>/dev/null || true
    cat > .git/hooks/pre-commit << 'EOF'
#!/bin/bash

# Pre-commit hook for Goethe Dialog System
# This hook runs before each commit to ensure code quality

echo "🔍 Running pre-commit checks..."

# Run the pre-commit verification script
if [ -f "scripts/pre-commit-verify.sh" ]; then
    bash scripts/pre-commit-verify.sh
    if [ $? -ne 0 ]; then
        echo "❌ Pre-commit verification failed. Please fix the issues before committing."
        exit 1
    fi
else
    echo "⚠️  Pre-commit verification script not found at scripts/pre-commit-verify.sh"
    echo "Continuing with commit..."
fi

echo "✅ Pre-commit checks passed"
exit 0
EOF
    chmod +x .git/hooks/pre-commit
    echo -e "${GREEN}✅${NC} Pre-commit hook installed"
else
    echo "⚠️  scripts/pre-commit-verify.sh not found"
fi

# Copy pre-push hook
if [ -f "scripts/pre-push-verify.sh" ]; then
    cp .git/hooks/pre-push .git/hooks/pre-push.backup 2>/dev/null || true
    cat > .git/hooks/pre-push << 'EOF'
#!/bin/bash

# Pre-push hook for Goethe Dialog System
# This hook runs before pushing to ensure the push will succeed

echo "🚀 Running pre-push checks..."

# Run the pre-push verification script
if [ -f "scripts/pre-push-verify.sh" ]; then
    bash scripts/pre-push-verify.sh
    if [ $? -ne 0 ]; then
        echo "❌ Pre-push verification failed. Please fix the issues before pushing."
        exit 1
    fi
else
    echo "⚠️  Pre-push verification script not found at scripts/pre-push-verify.sh"
    echo "Continuing with push..."
fi

echo "✅ Pre-push checks passed"
exit 0
EOF
    chmod +x .git/hooks/pre-push
    echo -e "${GREEN}✅${NC} Pre-push hook installed"
else
    echo "⚠️  scripts/pre-push-verify.sh not found"
fi

echo -e "${BLUE}🎉${NC} Git hooks installation completed!"
echo ""
echo "The following hooks are now active:"
echo "  - pre-commit: Runs before each commit"
echo "  - pre-push: Runs before each push"
echo ""
echo "To run verification manually:"
echo "  - bash scripts/pre-commit-verify.sh"
echo "  - bash scripts/pre-push-verify.sh"
echo ""
echo "To disable hooks temporarily:"
echo "  - git commit --no-verify"
echo "  - git push --no-verify"
