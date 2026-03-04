#!/bin/bash
# ==========================================================================
#  Octave Engine - Linux Uninstall Script
#
#  Removes Octave Engine from /opt/octave/ and cleans up system files.
#
#  Usage: sudo ./uninstall.sh
# ==========================================================================

set -e

INSTALL_DIR="/opt/octave"
WRAPPER="/usr/local/bin/octave-editor"
DESKTOP_FILE="/usr/share/applications/octave-editor.desktop"
MIME_FILE="/usr/share/mime/packages/octave-editor.xml"
ICON_FILE="/usr/share/icons/hicolor/128x128/apps/octave-editor.png"

# Check for root
if [ "$(id -u)" -ne 0 ]; then
    echo "ERROR: This script must be run as root (use sudo)."
    exit 1
fi

echo "============================================"
echo " Octave Engine - Linux Uninstaller"
echo "============================================"
echo ""

# --- Remove install directory ---
if [ -d "$INSTALL_DIR" ]; then
    echo "Removing $INSTALL_DIR..."
    rm -rf "$INSTALL_DIR"
else
    echo "  $INSTALL_DIR not found (already removed?)"
fi

# --- Remove wrapper script ---
if [ -f "$WRAPPER" ]; then
    echo "Removing wrapper script..."
    rm -f "$WRAPPER"
fi

# --- Remove desktop entry ---
if [ -f "$DESKTOP_FILE" ]; then
    echo "Removing desktop entry..."
    rm -f "$DESKTOP_FILE"
fi

# --- Remove MIME type ---
if [ -f "$MIME_FILE" ]; then
    echo "Removing MIME type..."
    rm -f "$MIME_FILE"
fi

# --- Remove icon ---
if [ -f "$ICON_FILE" ]; then
    echo "Removing icon..."
    rm -f "$ICON_FILE"
fi

# --- Update system caches ---
echo "Updating system caches..."
if command -v update-desktop-database > /dev/null 2>&1; then
    update-desktop-database /usr/share/applications 2>/dev/null || true
fi
if command -v update-mime-database > /dev/null 2>&1; then
    update-mime-database /usr/share/mime 2>/dev/null || true
fi
if command -v gtk-update-icon-cache > /dev/null 2>&1; then
    gtk-update-icon-cache /usr/share/icons/hicolor 2>/dev/null || true
fi

echo ""
echo "============================================"
echo " Uninstall complete."
echo "============================================"
