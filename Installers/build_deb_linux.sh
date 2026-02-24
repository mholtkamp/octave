#!/bin/bash
# ==========================================================================
#  build_deb_linux.sh
#  Builds a .deb package for Octave Engine.
#
#  Prerequisites:
#    - Python 3
#    - dpkg-deb (standard on Debian/Ubuntu)
#    - Engine built (OctaveEditor ELF binary)
#
#  Usage: bash Installers/build_deb_linux.sh
# ==========================================================================

set -e

# Navigate to repo root
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$REPO_ROOT"

echo "============================================"
echo " Octave Engine - Debian Package Builder"
echo "============================================"
echo ""

# --- Step 1: Stage distribution files ---
echo "[1/3] Staging distribution files..."
python3 Installers/stage_distribution.py --platform linux --verbose
echo ""

# --- Step 2: Read version ---
VERSION="$(grep -oP 'Version=\K.*' dist/Editor/version.txt | tr -d '[:space:]')"
PACKAGE_NAME="octave-engine"
DEB_DIR="dist/${PACKAGE_NAME}_${VERSION}_amd64"

echo "[2/3] Assembling .deb structure (version ${VERSION})..."

# Clean previous deb build
rm -rf "$DEB_DIR"

# --- Create directory structure ---
mkdir -p "$DEB_DIR/DEBIAN"
mkdir -p "$DEB_DIR/opt/octave"
mkdir -p "$DEB_DIR/usr/share/applications"
mkdir -p "$DEB_DIR/usr/share/mime/packages"
mkdir -p "$DEB_DIR/usr/share/icons/hicolor/128x128/apps"

# --- Copy staged files to /opt/octave ---
cp -r dist/Editor/* "$DEB_DIR/opt/octave/"

# --- Copy FHS integration files ---
cp Installers/Linux/octave-editor.desktop "$DEB_DIR/usr/share/applications/"
cp Installers/Linux/octave-editor.xml "$DEB_DIR/usr/share/mime/packages/"
if [ -f dist/Editor/OctaveLogo_128.png ]; then
    cp dist/Editor/OctaveLogo_128.png "$DEB_DIR/usr/share/icons/hicolor/128x128/apps/octave-editor.png"
fi

# --- DEBIAN/control ---
cat > "$DEB_DIR/DEBIAN/control" << EOF
Package: ${PACKAGE_NAME}
Version: ${VERSION}
Section: devel
Priority: optional
Architecture: amd64
Depends: libvulkan1, libxcb1, libasound2
Maintainer: Octave Engine <contact@example.com>
Description: Octave Game Engine Editor
 Multi-platform game engine with ImGui editor, Vulkan rendering,
 Lua scripting, visual node graphs, and timeline animation.
 Targets Windows, Linux, GameCube, Wii, and Nintendo 3DS.
EOF

# --- DEBIAN/postinst ---
cat > "$DEB_DIR/DEBIAN/postinst" << 'EOF'
#!/bin/bash
set -e

# Make binary executable
chmod +x /opt/octave/OctaveEditor

# Create writable directories for runtime output
mkdir -p /opt/octave/Engine/Saves
chmod 777 /opt/octave/Engine/Saves
mkdir -p /opt/octave/Standalone
chmod 777 /opt/octave/Standalone

# Create wrapper script that cds to install dir (required for path detection)
cat > /usr/local/bin/octave-editor << 'WRAPPER'
#!/bin/bash
cd /opt/octave
exec /opt/octave/OctaveEditor "$@"
WRAPPER
chmod +x /usr/local/bin/octave-editor

# Update system caches
if command -v update-desktop-database > /dev/null 2>&1; then
    update-desktop-database /usr/share/applications 2>/dev/null || true
fi
if command -v update-mime-database > /dev/null 2>&1; then
    update-mime-database /usr/share/mime 2>/dev/null || true
fi
if command -v gtk-update-icon-cache > /dev/null 2>&1; then
    gtk-update-icon-cache /usr/share/icons/hicolor 2>/dev/null || true
fi

exit 0
EOF
chmod 755 "$DEB_DIR/DEBIAN/postinst"

# --- DEBIAN/prerm ---
cat > "$DEB_DIR/DEBIAN/prerm" << 'EOF'
#!/bin/bash
set -e

# Remove wrapper script
rm -f /usr/local/bin/octave-editor

# Update system caches
if command -v update-desktop-database > /dev/null 2>&1; then
    update-desktop-database /usr/share/applications 2>/dev/null || true
fi
if command -v update-mime-database > /dev/null 2>&1; then
    update-mime-database /usr/share/mime 2>/dev/null || true
fi

exit 0
EOF
chmod 755 "$DEB_DIR/DEBIAN/prerm"

# --- Step 3: Build the .deb ---
echo "[3/3] Building .deb package..."
dpkg-deb --build "$DEB_DIR"

DEB_FILE="${DEB_DIR}.deb"
echo ""
echo "============================================"
echo " .deb package built successfully!"
echo " Output: $DEB_FILE"
echo ""
echo " Install: sudo dpkg -i $DEB_FILE"
echo "    or:   sudo apt install ./$DEB_FILE"
echo " Remove:  sudo apt remove ${PACKAGE_NAME}"
echo "============================================"
