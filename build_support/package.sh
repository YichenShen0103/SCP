#!/bin/bash

# package.sh - Cross-platform dependency installer for SCP compiler
# This script downloads and installs gtest and spim on various platforms

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Logging functions
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Detect the operating system
detect_os() {
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        if command -v apt-get >/dev/null 2>&1; then
            echo "ubuntu"
        elif command -v yum >/dev/null 2>&1; then
            echo "centos"
        elif command -v pacman >/dev/null 2>&1; then
            echo "arch"
        else
            echo "linux"
        fi
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        echo "macos"
    elif [[ "$OSTYPE" == "cygwin" ]] || [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "win32" ]]; then
        echo "windows"
    else
        echo "unknown"
    fi
}

# Check if a command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Install dependencies on Ubuntu/Debian
install_ubuntu() {
    log_info "Installing dependencies on Ubuntu/Debian..."
    
    # Update package list
    sudo apt-get update
    
    # Install gtest
    if ! dpkg -l | grep -q libgtest-dev; then
        log_info "Installing Google Test..."
        sudo apt-get install -y libgtest-dev cmake build-essential
        
        # Build and install gtest
        cd /usr/src/gtest
        sudo cmake CMakeLists.txt
        sudo make
        sudo cp lib/*.a /usr/lib
        cd - >/dev/null
        
        log_success "Google Test installed successfully"
    else
        log_info "Google Test is already installed"
    fi
    
    # Install spim
    if ! command_exists spim; then
        log_info "Installing SPIM..."
        sudo apt-get install -y spim
        log_success "SPIM installed successfully"
    else
        log_info "SPIM is already installed"
    fi
}

# Install dependencies on CentOS/RHEL/Fedora
install_centos() {
    log_info "Installing dependencies on CentOS/RHEL/Fedora..."
    
    # Determine package manager
    if command_exists dnf; then
        PKG_MANAGER="dnf"
    else
        PKG_MANAGER="yum"
    fi
    
    # Install development tools
    sudo $PKG_MANAGER groupinstall -y "Development Tools"
    sudo $PKG_MANAGER install -y cmake
    
    # Install gtest
    if ! rpm -qa | grep -q gtest; then
        log_info "Installing Google Test..."
        sudo $PKG_MANAGER install -y gtest-devel
        log_success "Google Test installed successfully"
    else
        log_info "Google Test is already installed"
    fi
    
    # Install spim (may need EPEL repository)
    if ! command_exists spim; then
        log_info "Installing SPIM..."
        # Enable EPEL repository for CentOS/RHEL
        if [[ "$PKG_MANAGER" == "yum" ]]; then
            sudo yum install -y epel-release
        fi
        sudo $PKG_MANAGER install -y spim
        log_success "SPIM installed successfully"
    else
        log_info "SPIM is already installed"
    fi
}

# Install dependencies on Arch Linux
install_arch() {
    log_info "Installing dependencies on Arch Linux..."
    
    # Update package database
    sudo pacman -Sy
    
    # Install gtest
    if ! pacman -Qs gtest >/dev/null; then
        log_info "Installing Google Test..."
        sudo pacman -S --noconfirm gtest cmake base-devel
        log_success "Google Test installed successfully"
    else
        log_info "Google Test is already installed"
    fi
    
    # Install spim
    if ! command_exists spim; then
        log_info "Installing SPIM..."
        sudo pacman -S --noconfirm spim
        log_success "SPIM installed successfully"
    else
        log_info "SPIM is already installed"
    fi
}

# Install dependencies on macOS
install_macos() {
    log_info "Installing dependencies on macOS..."
    
    # Check if Homebrew is installed
    if ! command_exists brew; then
        log_error "Homebrew is not installed. Please install Homebrew first:"
        log_error "  /bin/bash -c \"\$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)\""
        exit 1
    fi
    
    # Update Homebrew
    brew update
    
    # Install gtest
    if ! brew list googletest >/dev/null 2>&1; then
        log_info "Installing Google Test..."
        brew install googletest
        log_success "Google Test installed successfully"
    else
        log_info "Google Test is already installed"
    fi
    
    # Install spim
    if ! command_exists spim; then
        log_info "Installing SPIM..."
        brew install spim
        log_success "SPIM installed successfully"
    else
        log_info "SPIM is already installed"
    fi
}

# Install dependencies on Windows (using MSYS2)
install_windows() {
    log_info "Installing dependencies on Windows..."
    
    if command_exists pacman; then
        # MSYS2 environment
        log_info "Detected MSYS2 environment"
        
        # Update package database
        pacman -Sy
        
        # Install gtest
        log_info "Installing Google Test..."
        pacman -S --noconfirm mingw-w64-x86_64-gtest mingw-w64-x86_64-cmake mingw-w64-x86_64-toolchain
        
        # Note: SPIM might not be available on Windows through MSYS2
        log_warning "SPIM might not be available on Windows. Consider using QtSpim or alternative MIPS simulators."
        log_info "You can download QtSpim from: http://spimsimulator.sourceforge.net/"
        
    else
        log_error "Windows installation requires MSYS2 or similar Unix-like environment"
        log_error "Please install MSYS2 from: https://www.msys2.org/"
        exit 1
    fi
}

# Build gtest from source (fallback method)
build_gtest_from_source() {
    log_info "Building Google Test from source..."
    
    TEMP_DIR=$(mktemp -d)
    cd "$TEMP_DIR"
    
    # Clone googletest repository
    git clone https://github.com/google/googletest.git
    cd googletest
    
    # Create build directory
    mkdir build
    cd build
    
    # Configure and build
    cmake ..
    make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
    
    # Install
    sudo make install
    
    # Clean up
    cd - >/dev/null
    rm -rf "$TEMP_DIR"
    
    log_success "Google Test built and installed from source"
}

# Verify installations
verify_installation() {
    log_info "Verifying installations..."
    
    # Check gtest
    if [ -f "/usr/local/lib/libgtest.a" ] || [ -f "/usr/lib/libgtest.a" ] || pkg-config --exists gtest 2>/dev/null; then
        log_success "Google Test is properly installed"
    else
        log_warning "Google Test installation could not be verified"
    fi
    
    # Check spim
    if command_exists spim; then
        SPIM_VERSION=$(spim -version 2>&1 | head -n1 || echo "Version unknown")
        log_success "SPIM is properly installed: $SPIM_VERSION"
    else
        log_warning "SPIM installation could not be verified"
    fi
}

# Main installation function
main() {
    log_info "SCP Compiler Dependency Installer"
    log_info "================================"
    
    OS=$(detect_os)
    log_info "Detected operating system: $OS"
    
    case "$OS" in
        "ubuntu")
            install_ubuntu
            ;;
        "centos")
            install_centos
            ;;
        "arch")
            install_arch
            ;;
        "macos")
            install_macos
            ;;
        "windows")
            install_windows
            ;;
        "linux")
            log_warning "Generic Linux detected. Attempting to build from source..."
            build_gtest_from_source
            log_warning "Please install SPIM manually for your distribution"
            ;;
        *)
            log_error "Unsupported operating system: $OS"
            log_error "Please install dependencies manually:"
            log_error "  - Google Test: https://github.com/google/googletest"
            log_error "  - SPIM: http://spimsimulator.sourceforge.net/"
            exit 1
            ;;
    esac
    
    verify_installation
    
    log_success "Dependency installation completed!"
    log_info "You can now build the SCP compiler with:"
    log_info "  mkdir -p build && cd build"
    log_info "  cmake .."
    log_info "  make"
}

# Handle command line arguments
case "${1:-}" in
    "--help" | "-h")
        echo "Usage: $0 [--help]"
        echo ""
        echo "This script installs the required dependencies for the SCP compiler:"
        echo "  - Google Test (gtest) for unit testing"
        echo "  - SPIM for MIPS simulation"
        echo ""
        echo "Supported platforms:"
        echo "  - Ubuntu/Debian (apt)"
        echo "  - CentOS/RHEL/Fedora (yum/dnf)"
        echo "  - Arch Linux (pacman)"
        echo "  - macOS (brew)"
        echo "  - Windows (MSYS2)"
        echo ""
        echo "Options:"
        echo "  --help, -h    Show this help message"
        exit 0
        ;;
    "")
        main
        ;;
    *)
        log_error "Unknown option: $1"
        log_error "Use --help for usage information"
        exit 1
        ;;
esac

