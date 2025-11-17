#!/bin/bash
# Quick Start Script for Providers Agent

set -e  # Exit on error

echo "🚀 Providers Agent - Quick Start"
echo "================================"
echo ""

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ]; then
    echo "❌ Error: Please run this script from the providers-agent directory"
    exit 1
fi

# Check dependencies
echo "📦 Checking dependencies..."

if ! command -v cmake &> /dev/null; then
    echo "❌ CMake not found. Installing..."
    brew install cmake
else
    echo "✅ CMake found: $(cmake --version | head -1)"
fi

if ! command -v curl &> /dev/null; then
    echo "❌ curl not found. Installing..."
    brew install curl
else
    echo "✅ curl found"
fi

if ! brew list qt@6 &> /dev/null; then
    echo "❌ Qt6 not found. Installing..."
    brew install qt@6
else
    echo "✅ Qt6 found"
fi

echo ""
echo "🔨 Building application..."
echo ""

# Create build directory
mkdir -p build
cd build

# Configure
echo "Configuring CMake..."
cmake .. -DCMAKE_PREFIX_PATH=$(brew --prefix qt@6) || {
    echo "❌ CMake configuration failed"
    exit 1
}

# Build
echo "Building..."
cmake --build . -j$(sysctl -n hw.ncpu) || {
    echo "❌ Build failed"
    exit 1
}

echo ""
echo "✅ Build successful!"
echo ""

# Check if API endpoint is set
if [ -z "$HEART_BEAT_API_ENDPOINT" ]; then
    echo "⚠️  Warning: HEART_BEAT_API_ENDPOINT not set"
    echo "   Set it with: export HEART_BEAT_API_ENDPOINT=https://your-api-url.com"
    echo ""
    read -p "Enter API endpoint (or press Enter to skip): " api_endpoint
    if [ ! -z "$api_endpoint" ]; then
        export HEART_BEAT_API_ENDPOINT="$api_endpoint"
    fi
fi

# Check if executable exists
if [ ! -f "bin/providers_agent" ]; then
    echo "❌ Error: Executable not found after build"
    exit 1
fi

echo "🎉 Ready to run!"
echo ""
echo "To run the application:"
echo "  cd build"
echo "  export HEART_BEAT_API_ENDPOINT=https://your-api-url.com"
echo "  ./bin/providers_agent"
echo ""
read -p "Run now? (y/n): " -n 1 -r
echo ""

if [[ $REPLY =~ ^[Yy]$ ]]; then
    if [ -z "$HEART_BEAT_API_ENDPOINT" ]; then
        echo "⚠️  Running without API endpoint (heartbeat will fail)"
    fi
    echo "🚀 Starting Providers Agent..."
    ./bin/providers_agent
fi

