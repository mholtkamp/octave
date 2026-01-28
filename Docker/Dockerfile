# OctaveGameEngine - Universal Build Environment
#
# The editor is pre-compiled during image build and runs in headless mode
# to cook/package assets for each target platform.
#
# Usage:
#   Build image:
#     docker build -t octavegameengine -f Docker/Dockerfile .
#
#   Build Editor (copy pre-built editor to output):
#     docker run --rm -v ./output:/game octavegameengine build-editor
#
#   Build Game (requires project with .octp file):
#     docker run --rm -v ./output:/game -v ./MyGame:/project octavegameengine build-linux
#     docker run --rm -v ./output:/game -v ./MyGame:/project octavegameengine build-3ds
#     docker run --rm -v ./output:/game -v ./MyGame:/project octavegameengine build-gcn
#     docker run --rm -v ./output:/game -v ./MyGame:/project octavegameengine build-wii
#
#   Interactive:
#     docker run -it --rm -v ./output:/game -v ./MyGame:/project octavegameengine bash

FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

# Install base dependencies
RUN apt-get update && apt-get install -y \
    g++ \
    make \
    libx11-dev \
    libasound2-dev \
    wget \
    curl \
    ca-certificates \
    git \
    pkg-config \
    libvorbis-dev \
    xz-utils \
    sudo \
    gnupg \
    && rm -rf /var/lib/apt/lists/*

# Install Vulkan SDK 1.3.275.0
ENV VULKAN_VERSION=1.3.275.0
WORKDIR /tmp
RUN wget -q https://sdk.lunarg.com/sdk/download/${VULKAN_VERSION}/linux/vulkansdk-linux-x86_64-${VULKAN_VERSION}.tar.xz \
    && mkdir -p /opt/vulkan \
    && tar -xf vulkansdk-linux-x86_64-${VULKAN_VERSION}.tar.xz -C /opt/vulkan \
    && rm vulkansdk-linux-x86_64-${VULKAN_VERSION}.tar.xz

ENV VULKAN_SDK=/opt/vulkan/${VULKAN_VERSION}/x86_64
ENV PATH="${VULKAN_SDK}/bin:${PATH}"
ENV LD_LIBRARY_PATH="${VULKAN_SDK}/lib"
ENV VK_LAYER_PATH="${VULKAN_SDK}/share/vulkan/explicit_layer.d"

# Install devkitPro (for 3DS, GameCube, Wii)
ENV DEVKITPRO=/opt/devkitpro
ENV DEVKITPPC=${DEVKITPRO}/devkitPPC
ENV DEVKITARM=${DEVKITPRO}/devkitARM
ENV PATH="${DEVKITPRO}/tools/bin:${DEVKITPPC}/bin:${DEVKITARM}/bin:${PATH}"

# Install devkitPro pacman (using their required User-Agent "dkp apt")
RUN apt-get update && \
    apt-get install -y apt-transport-https && \
    mkdir -p /usr/share/keyring/ && \
    wget -U "dkp apt" -O /usr/share/keyring/devkitpro-pub.gpg https://apt.devkitpro.org/devkitpro-pub.gpg && \
    echo "deb [signed-by=/usr/share/keyring/devkitpro-pub.gpg] https://apt.devkitpro.org stable main" > /etc/apt/sources.list.d/devkitpro.list && \
    apt-get update && \
    apt-get install -y devkitpro-pacman && \
    rm -rf /var/lib/apt/lists/*

# Install console development libraries
# Create /etc/mtab symlink needed by pacman in Docker
RUN ln -sf /proc/self/mounts /etc/mtab && \
    dkp-pacman -Syu --noconfirm && \
    dkp-pacman -S --noconfirm gamecube-dev wii-dev 3ds-dev

# Create directories
RUN mkdir -p /octave /game /project
WORKDIR /octave

# Copy source
COPY . /octave/

# Initialize submodules if needed
RUN if [ -f .gitmodules ]; then \
        git config --global --add safe.directory /octave && \
        git submodule update --init --recursive || true; \
    fi

# Compile shaders
WORKDIR /octave/Engine/Shaders/GLSL
RUN chmod +x compile.sh && ./compile.sh

WORKDIR /octave

# ============================================================================
# Pre-build Linux Editor (for headless asset cooking)
# ============================================================================
RUN cd /octave/Standalone && make -j$(nproc) -f Makefile_Linux_Editor
RUN cp /octave/Standalone/Build/Linux/OctaveEditor.elf /octave/OctaveEditor.elf

# ============================================================================
# Build Commands
# ============================================================================

# build-editor: Build the Linux Editor (and optionally copy to output)
RUN echo '#!/bin/bash\n\
set -e\n\
\n\
echo "Linux Editor already pre-built at /octave/OctaveEditor.elf"\n\
\n\
# Check for project (optional)\n\
if [ -d "/project" ] && [ -n "$(ls -A /project 2>/dev/null)" ]; then\n\
    OCTP_FILE=$(find /project -maxdepth 1 -name "*.octp" -print -quit)\n\
    echo "Project: /project"\n\
    [ -n "$OCTP_FILE" ] && echo "Project file: $OCTP_FILE"\n\
fi\n\
\n\
echo "Copying editor to /game..."\n\
cp /octave/OctaveEditor.elf /game/\n\
cp -r /octave/Engine /game/\n\
\n\
echo "Done! Editor at /game/OctaveEditor.elf"\n\
' > /usr/local/bin/build-editor && chmod +x /usr/local/bin/build-editor

# build-linux: Build Linux Game with embedded assets
RUN echo '#!/bin/bash\n\
set -e\n\
\n\
if [ ! -d "/project" ] || [ -z "$(ls -A /project 2>/dev/null)" ]; then\n\
    echo "Error: No project mounted at /project"\n\
    echo "Usage: docker run --rm -v ./output:/game -v ./MyGame:/project octavegameengine build-linux"\n\
    exit 1\n\
fi\n\
\n\
OCTP_FILE=$(find /project -maxdepth 1 -name "*.octp" -print -quit)\n\
if [ -z "$OCTP_FILE" ]; then\n\
    echo "Error: No .octp project file found in /project"\n\
    exit 1\n\
fi\n\
\n\
echo "Building Linux Game..."\n\
echo "Project: /project"\n\
echo "Project file: $OCTP_FILE"\n\
\n\
# Copy project to octave directory (editor expects paths relative to working dir)\n\
PROJECT_NAME=$(basename "$OCTP_FILE" .octp)\n\
rm -rf /octave/Standalone 2>/dev/null || true\n\
mkdir -p /octave/Standalone\n\
cp -r /project/* /octave/Standalone/\n\
\n\
# Cook assets using headless editor\n\
echo "Cooking assets for Linux (embedded)..."\n\
cd /octave\n\
./OctaveEditor.elf -headless -project "Standalone/${PROJECT_NAME}.octp" -build Linux embedded\n\
\n\
# Build the game executable\n\
echo "Building game executable..."\n\
cd /octave/Standalone\n\
make -j$(nproc) -f Makefile_Linux_Game\n\
\n\
echo "Copying to /game..."\n\
cp /octave/Standalone/Build/Linux/* /game/\n\
\n\
echo "Done! Output at /game/"\n\
' > /usr/local/bin/build-linux && chmod +x /usr/local/bin/build-linux

# build-3ds: Build for Nintendo 3DS with embedded assets
RUN echo '#!/bin/bash\n\
set -e\n\
\n\
if ! command -v arm-none-eabi-gcc &> /dev/null; then\n\
    echo "Error: devkitARM not installed. Console builds unavailable."\n\
    exit 1\n\
fi\n\
\n\
if [ ! -d "/project" ] || [ -z "$(ls -A /project 2>/dev/null)" ]; then\n\
    echo "Error: No project mounted at /project"\n\
    echo "Usage: docker run --rm -v ./output:/game -v ./MyGame:/project octavegameengine build-3ds"\n\
    exit 1\n\
fi\n\
\n\
OCTP_FILE=$(find /project -maxdepth 1 -name "*.octp" -print -quit)\n\
if [ -z "$OCTP_FILE" ]; then\n\
    echo "Error: No .octp project file found in /project"\n\
    exit 1\n\
fi\n\
\n\
echo "Building for Nintendo 3DS..."\n\
echo "Project: /project"\n\
echo "Project file: $OCTP_FILE"\n\
\n\
# Copy project to octave directory\n\
PROJECT_NAME=$(basename "$OCTP_FILE" .octp)\n\
rm -rf /octave/Standalone 2>/dev/null || true\n\
mkdir -p /octave/Standalone\n\
cp -r /project/* /octave/Standalone/\n\
\n\
# Cook assets using headless editor\n\
echo "Cooking assets for 3DS (embedded)..."\n\
cd /octave\n\
./OctaveEditor.elf -headless -project "Standalone/${PROJECT_NAME}.octp" -build 3DS embedded\n\
\n\
# Build the game executable\n\
echo "Building 3DS executable..."\n\
cd /octave/Standalone\n\
make -j$(nproc) -f Makefile_3DS\n\
\n\
echo "Copying ROM to /game..."\n\
cp /octave/Standalone/Build/3DS/* /game/\n\
\n\
echo "Done! 3DS build at /game/"\n\
' > /usr/local/bin/build-3ds && chmod +x /usr/local/bin/build-3ds

# build-gcn: Build for Nintendo GameCube with embedded assets
RUN echo '#!/bin/bash\n\
set -e\n\
\n\
if ! command -v powerpc-eabi-gcc &> /dev/null; then\n\
    echo "Error: devkitPPC not installed. Console builds unavailable."\n\
    exit 1\n\
fi\n\
\n\
if [ ! -d "/project" ] || [ -z "$(ls -A /project 2>/dev/null)" ]; then\n\
    echo "Error: No project mounted at /project"\n\
    echo "Usage: docker run --rm -v ./output:/game -v ./MyGame:/project octavegameengine build-gcn"\n\
    exit 1\n\
fi\n\
\n\
OCTP_FILE=$(find /project -maxdepth 1 -name "*.octp" -print -quit)\n\
if [ -z "$OCTP_FILE" ]; then\n\
    echo "Error: No .octp project file found in /project"\n\
    exit 1\n\
fi\n\
\n\
echo "Building for Nintendo GameCube..."\n\
echo "Project: /project"\n\
echo "Project file: $OCTP_FILE"\n\
\n\
# Copy project to octave directory\n\
PROJECT_NAME=$(basename "$OCTP_FILE" .octp)\n\
rm -rf /octave/Standalone 2>/dev/null || true\n\
mkdir -p /octave/Standalone\n\
cp -r /project/* /octave/Standalone/\n\
\n\
# Cook assets using headless editor\n\
echo "Cooking assets for GameCube (embedded)..."\n\
cd /octave\n\
./OctaveEditor.elf -headless -project "Standalone/${PROJECT_NAME}.octp" -build GameCube embedded\n\
\n\
# Build the game executable\n\
echo "Building GameCube executable..."\n\
cd /octave/Standalone\n\
make -j$(nproc) -f Makefile_GCN\n\
\n\
echo "Copying ROM to /game..."\n\
cp /octave/Standalone/Build/GCN/* /game/\n\
\n\
echo "Done! GameCube build at /game/"\n\
' > /usr/local/bin/build-gcn && chmod +x /usr/local/bin/build-gcn

# build-wii: Build for Nintendo Wii with embedded assets
RUN echo '#!/bin/bash\n\
set -e\n\
\n\
if ! command -v powerpc-eabi-gcc &> /dev/null; then\n\
    echo "Error: devkitPPC not installed. Console builds unavailable."\n\
    exit 1\n\
fi\n\
\n\
if [ ! -d "/project" ] || [ -z "$(ls -A /project 2>/dev/null)" ]; then\n\
    echo "Error: No project mounted at /project"\n\
    echo "Usage: docker run --rm -v ./output:/game -v ./MyGame:/project octavegameengine build-wii"\n\
    exit 1\n\
fi\n\
\n\
OCTP_FILE=$(find /project -maxdepth 1 -name "*.octp" -print -quit)\n\
if [ -z "$OCTP_FILE" ]; then\n\
    echo "Error: No .octp project file found in /project"\n\
    exit 1\n\
fi\n\
\n\
echo "Building for Nintendo Wii..."\n\
echo "Project: /project"\n\
echo "Project file: $OCTP_FILE"\n\
\n\
# Copy project to octave directory\n\
PROJECT_NAME=$(basename "$OCTP_FILE" .octp)\n\
rm -rf /octave/Standalone 2>/dev/null || true\n\
mkdir -p /octave/Standalone\n\
cp -r /project/* /octave/Standalone/\n\
\n\
# Cook assets using headless editor\n\
echo "Cooking assets for Wii (embedded)..."\n\
cd /octave\n\
./OctaveEditor.elf -headless -project "Standalone/${PROJECT_NAME}.octp" -build Wii embedded\n\
\n\
# Build the game executable\n\
echo "Building Wii executable..."\n\
cd /octave/Standalone\n\
make -j$(nproc) -f Makefile_Wii\n\
\n\
echo "Copying ROM to /game..."\n\
cp /octave/Standalone/Build/Wii/* /game/\n\
\n\
echo "Done! Wii build at /game/"\n\
' > /usr/local/bin/build-wii && chmod +x /usr/local/bin/build-wii

# Help command
RUN echo '#!/bin/bash\n\
echo "OctaveGameEngine Build Environment"\n\
echo "==================================="\n\
echo ""\n\
echo "The editor is pre-built and uses headless mode to cook assets."\n\
echo ""\n\
echo "Commands:"\n\
echo "  build-editor  - Copy pre-built Linux Editor to output"\n\
echo "  build-linux   - Build Linux Game with embedded assets"\n\
echo "  build-3ds     - Build Nintendo 3DS with embedded assets"\n\
echo "  build-gcn     - Build Nintendo GameCube with embedded assets"\n\
echo "  build-wii     - Build Nintendo Wii with embedded assets"\n\
echo ""\n\
echo "Volumes:"\n\
echo "  /game     - Output directory (mount with -v ./output:/game)"\n\
echo "  /project  - Your game project with .octp file (mount with -v ./MyGame:/project)"\n\
echo ""\n\
echo "Examples:"\n\
echo "  docker run --rm -v ./output:/game octavegameengine build-editor"\n\
echo "  docker run --rm -v ./output:/game -v ./MyGame:/project octavegameengine build-linux"\n\
echo "  docker run --rm -v ./output:/game -v ./MyGame:/project octavegameengine build-wii"\n\
' > /usr/local/bin/help && chmod +x /usr/local/bin/help

VOLUME /game
VOLUME /project

ENTRYPOINT []
CMD ["help"]
