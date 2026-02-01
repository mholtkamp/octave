#!/bin/bash
#
# OctaveGameEngine Docker Build Script
#
# Usage:
#   ./Docker/build.sh           - Build the Docker image
#   ./Docker/build.sh --help    - Show help
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OCTAVE_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
IMAGE_NAME="octavegameengine"

cd "$OCTAVE_ROOT"

case "${1:-build}" in
    build|"")
        echo "Building ${IMAGE_NAME} Docker image..."
        docker build -f Docker/Dockerfile -t "${IMAGE_NAME}" .
        echo ""
        echo "Done! Image: ${IMAGE_NAME}"
        echo ""
        echo "Usage:"
        echo "  docker run --rm -v ./output:/game ${IMAGE_NAME} build-editor"
        echo "  docker run --rm -v ./output:/game ${IMAGE_NAME} build-linux"
        echo "  docker run --rm -v ./output:/game ${IMAGE_NAME} build-3ds"
        echo "  docker run --rm -v ./output:/game ${IMAGE_NAME} build-gcn"
        echo "  docker run --rm -v ./output:/game ${IMAGE_NAME} build-wii"
        ;;
    help|--help|-h)
        echo "OctaveGameEngine Docker Build Script"
        echo ""
        echo "Usage: $0 [command]"
        echo ""
        echo "Commands:"
        echo "  build (default)  - Build the Docker image"
        echo "  help             - Show this help"
        echo ""
        echo "After building, run:"
        echo "  docker run --rm -v ./output:/game ${IMAGE_NAME} build-editor"
        echo "  docker run --rm -v ./output:/game ${IMAGE_NAME} build-linux"
        echo "  docker run --rm -v ./output:/game ${IMAGE_NAME} build-3ds"
        echo "  docker run --rm -v ./output:/game ${IMAGE_NAME} build-gcn"
        echo "  docker run --rm -v ./output:/game ${IMAGE_NAME} build-wii"
        ;;
    *)
        echo "Unknown command: $1"
        echo "Run '$0 help' for usage information"
        exit 1
        ;;
esac
