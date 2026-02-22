#!/bin/bash
# ==========================================================================
#  build_tarball_linux.sh
#  Builds a .tar.gz distribution for Octave Engine (non-Debian distros).
#
#  Prerequisites:
#    - Python 3
#    - Engine built (OctaveEditor ELF binary)
#
#  Usage: bash Installers/build_tarball_linux.sh
# ==========================================================================

set -e

# Navigate to repo root
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$REPO_ROOT"

echo "============================================"
echo " Octave Engine - Tarball Builder"
echo "============================================"
echo ""

# --- Step 1: Stage distribution files ---
echo "[1/3] Staging distribution files..."
python3 Installers/stage_distribution.py --platform linux --verbose
echo ""

# --- Step 2: Read version ---
VERSION="$(grep -oP 'Version=\K.*' dist/Editor/version.txt | tr -d '[:space:]')"
TARBALL_NAME="OctaveEditor-linux-x64"
TARBALL_DIR="dist/${TARBALL_NAME}"

echo "[2/3] Assembling tarball structure (version ${VERSION})..."

# Clean previous tarball build
rm -rf "$TARBALL_DIR"
mkdir -p "$TARBALL_DIR"

# --- Copy staged files into octave/ subdirectory ---
cp -r dist/Editor "$TARBALL_DIR/octave"

# --- Copy install/uninstall scripts and desktop files ---
cp Installers/Linux/install.sh "$TARBALL_DIR/"
cp Installers/Linux/uninstall.sh "$TARBALL_DIR/"
cp Installers/Linux/octave-editor.desktop "$TARBALL_DIR/"
cp Installers/Linux/octave-editor.xml "$TARBALL_DIR/"

# Make scripts executable
chmod +x "$TARBALL_DIR/install.sh"
chmod +x "$TARBALL_DIR/uninstall.sh"
chmod +x "$TARBALL_DIR/octave/OctaveEditor" 2>/dev/null || true

# --- Step 3: Create tarball ---
echo "[3/3] Creating tarball..."
cd dist
tar -czf "${TARBALL_NAME}.tar.gz" "${TARBALL_NAME}/"
cd "$REPO_ROOT"

TARBALL_FILE="dist/${TARBALL_NAME}.tar.gz"
echo ""
echo "============================================"
echo " Tarball built successfully!"
echo " Output: $TARBALL_FILE"
echo ""
echo " Install:"
echo "   tar -xzf ${TARBALL_NAME}.tar.gz"
echo "   cd ${TARBALL_NAME}"
echo "   sudo ./install.sh"
echo ""
echo " Uninstall:"
echo "   sudo ./uninstall.sh"
echo "============================================"
