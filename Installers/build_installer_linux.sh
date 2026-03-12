#!/bin/bash
# ==========================================================================
#  build_installer_linux.sh
#  Full build pipeline for Octave Engine Linux 64-bit packages.
#
#  This script performs the complete build process:
#    1. Initialize git submodules
#    2. Compile shaders
#    3. Build Engine (Linux Editor)
#    4. Build .deb package
#    5. Build tarball
#
#  Prerequisites:
#    - GCC/G++ toolchain
#    - Vulkan SDK installed (VULKAN_SDK environment variable set)
#    - Python 3
#    - System packages: libxcb1-dev libasound2-dev libx11-dev dpkg-dev
#
#  Usage: bash Installers/build_installer_linux.sh
# ==========================================================================

set -e

# Navigate to repo root
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$REPO_ROOT"

echo "============================================"
echo " Octave Engine - Linux 64-bit Full Build"
echo "============================================"
echo ""

# --- Check prerequisites ---
echo "Checking prerequisites..."

if [ -z "$VULKAN_SDK" ]; then
    echo "ERROR: VULKAN_SDK environment variable not set."
    echo "       Install the Vulkan SDK and set VULKAN_SDK."
    exit 1
fi
echo "  [OK] VULKAN_SDK = $VULKAN_SDK"

if ! command -v python3 &> /dev/null; then
    echo "ERROR: Python 3 not found."
    exit 1
fi
echo "  [OK] Python 3 found"

if ! command -v g++ &> /dev/null; then
    echo "ERROR: g++ not found. Install build-essential."
    exit 1
fi
echo "  [OK] g++ found"

if ! command -v dpkg-deb &> /dev/null; then
    echo "WARNING: dpkg-deb not found. .deb package will not be built."
    echo "         Install with: sudo apt-get install dpkg-dev"
    SKIP_DEB=1
fi

echo ""

# --- Step 1: Initialize submodules ---
echo "[1/5] Initializing git submodules..."
git submodule init -- External/bullet3 External/doxygen-awesome-css External/zep Plugins/Blender/octave-gameengine-connect
git submodule update --recursive
echo "  Submodules initialized."
echo ""

# --- Step 2: Compile shaders ---
echo "[2/5] Compiling shaders..."
pushd Engine/Shaders/GLSL > /dev/null
chmod +x compile.sh
./compile.sh
popd > /dev/null
echo "  Shaders compiled."
echo ""

# --- Step 3: Build Engine ---
echo "[3/5] Building Engine (Linux Editor)..."
NPROC=$(nproc 2>/dev/null || echo 4)
make -C Standalone -f Makefile_Linux_Editor -j$NPROC
echo "  Engine built successfully."
echo ""

# --- Step 4: Build .deb package ---
if [ -z "$SKIP_DEB" ]; then
    echo "[4/5] Building .deb package..."
    bash Installers/build_deb_linux.sh
    echo ""
else
    echo "[4/5] Skipping .deb package (dpkg-deb not available)."
    echo ""
fi

# --- Step 5: Build tarball ---
echo "[5/5] Building tarball..."
bash Installers/build_tarball_linux.sh
echo ""

echo "============================================"
echo " BUILD COMPLETE!"
echo "============================================"
echo ""
echo " Outputs in dist/:"
ls -la dist/*.deb dist/*.tar.gz 2>/dev/null || echo "  (check dist/ folder)"
echo ""
echo "============================================"
