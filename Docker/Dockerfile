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
#   Debug mode (drops to shell on failure):
#     docker run -it --rm -v ./output:/game -v ./MyGame:/project octavegameengine build-3ds --debug
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

# Create libogc2 symlink (project makefiles expect this path)
RUN ln -sf ${DEVKITPPC} ${DEVKITPRO}/libogc2

# Create directories
RUN mkdir -p /octave /game /project /intermediate
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
chmod -R 777 /game\n\
\n\
echo "Done! Editor at /game/OctaveEditor.elf"\n\
' > /usr/local/bin/build-editor && chmod +x /usr/local/bin/build-editor

# build-linux: Build Linux Game with embedded assets
RUN echo '#!/bin/bash\n\
\n\
DEBUG=0\n\
for arg in "$@"; do\n\
    case $arg in\n\
        --debug|-d) DEBUG=1 ;;\n\
    esac\n\
done\n\
\n\
handle_error() {\n\
    echo ""\n\
    echo "=== BUILD FAILED ==="\n\
    if [ $DEBUG -eq 1 ]; then\n\
        echo "Debug mode: dropping to shell. Type exit to quit."\n\
        exec /bin/bash\n\
    fi\n\
    exit 1\n\
}\n\
trap handle_error ERR\n\
set -e\n\
\n\
if [ ! -d "/project" ] || [ -z "$(ls -A /project 2>/dev/null)" ]; then\n\
    echo "Error: No project mounted at /project"\n\
    echo "Usage: docker run -it --rm -v ./output:/game -v ./MyGame:/project octavegameengine build-linux [--debug]"\n\
    exit 1\n\
fi\n\
\n\
OCTP_FILE=$(find /project -maxdepth 1 -name "*.octp" -print -quit)\n\
if [ -z "$OCTP_FILE" ]; then\n\
    echo "Error: No .octp project file found in /project"\n\
    exit 1\n\
fi\n\
\n\
PROJECT_NAME=$(basename "$OCTP_FILE" .octp)\n\
echo "Building Linux Game..."\n\
echo "Project: /project"\n\
echo "Project file: $OCTP_FILE"\n\
[ $DEBUG -eq 1 ] && echo "Debug mode: enabled"\n\
\n\
# Copy project to intermediate directory to protect original\n\
echo "Copying project to intermediate directory..."\n\
rm -rf /intermediate/*\n\
cp -r /project/* /intermediate/\n\
OCTP_FILE="/intermediate/$(basename $OCTP_FILE)"\n\
\n\
# Build using headless editor (cooks assets and compiles)\n\
cd /octave\n\
./OctaveEditor.elf -headless -project "$OCTP_FILE" -build Linux embedded\n\
\n\
echo "Copying to /game..."\n\
cp -r /intermediate/Packaged/Linux/* /game/\n\
chmod -R 777 /game\n\
\n\
echo "Done! Output at /game/"\n\
' > /usr/local/bin/build-linux && chmod +x /usr/local/bin/build-linux

# build-3ds: Build for Nintendo 3DS with embedded assets
RUN echo '#!/bin/bash\n\
\n\
DEBUG=0\n\
for arg in "$@"; do\n\
    case $arg in\n\
        --debug|-d) DEBUG=1 ;;\n\
    esac\n\
done\n\
\n\
handle_error() {\n\
    echo ""\n\
    echo "=== BUILD FAILED ==="\n\
    if [ $DEBUG -eq 1 ]; then\n\
        echo "Debug mode: dropping to shell. Type exit to quit."\n\
        exec /bin/bash\n\
    fi\n\
    exit 1\n\
}\n\
trap handle_error ERR\n\
set -e\n\
\n\
if ! command -v arm-none-eabi-gcc &> /dev/null; then\n\
    echo "Error: devkitARM not installed. Console builds unavailable."\n\
    exit 1\n\
fi\n\
\n\
if [ ! -d "/project" ] || [ -z "$(ls -A /project 2>/dev/null)" ]; then\n\
    echo "Error: No project mounted at /project"\n\
    echo "Usage: docker run -it --rm -v ./output:/game -v ./MyGame:/project octavegameengine build-3ds [--debug]"\n\
    exit 1\n\
fi\n\
\n\
OCTP_FILE=$(find /project -maxdepth 1 -name "*.octp" -print -quit)\n\
if [ -z "$OCTP_FILE" ]; then\n\
    echo "Error: No .octp project file found in /project"\n\
    exit 1\n\
fi\n\
\n\
PROJECT_NAME=$(basename "$OCTP_FILE" .octp)\n\
echo "Building for Nintendo 3DS..."\n\
echo "Project: /project"\n\
echo "Project file: $OCTP_FILE"\n\
[ $DEBUG -eq 1 ] && echo "Debug mode: enabled"\n\
\n\
# Copy project to intermediate directory to protect original\n\
echo "Copying project to intermediate directory..."\n\
rm -rf /intermediate/*\n\
cp -r /project/* /intermediate/\n\
OCTP_FILE="/intermediate/$(basename $OCTP_FILE)"\n\
\n\
# Build using headless editor (cooks assets and compiles)\n\
cd /octave\n\
./OctaveEditor.elf -headless -project "$OCTP_FILE" -build 3DS embedded\n\
\n\
echo "Copying ROM to /game..."\n\
cp /intermediate/Packaged/3DS/*.3dsx /game/\n\
chmod -R 777 /game\n\
\n\
echo "Done! 3DS build at /game/"\n\
' > /usr/local/bin/build-3ds && chmod +x /usr/local/bin/build-3ds

# build-gcn: Build for Nintendo GameCube with embedded assets
RUN echo '#!/bin/bash\n\
\n\
DEBUG=0\n\
for arg in "$@"; do\n\
    case $arg in\n\
        --debug|-d) DEBUG=1 ;;\n\
    esac\n\
done\n\
\n\
handle_error() {\n\
    echo ""\n\
    echo "=== BUILD FAILED ==="\n\
    if [ $DEBUG -eq 1 ]; then\n\
        echo "Debug mode: dropping to shell. Type exit to quit."\n\
        exec /bin/bash\n\
    fi\n\
    exit 1\n\
}\n\
trap handle_error ERR\n\
set -e\n\
\n\
if ! command -v powerpc-eabi-gcc &> /dev/null; then\n\
    echo "Error: devkitPPC not installed. Console builds unavailable."\n\
    exit 1\n\
fi\n\
\n\
if [ ! -d "/project" ] || [ -z "$(ls -A /project 2>/dev/null)" ]; then\n\
    echo "Error: No project mounted at /project"\n\
    echo "Usage: docker run -it --rm -v ./output:/game -v ./MyGame:/project octavegameengine build-gcn [--debug]"\n\
    exit 1\n\
fi\n\
\n\
OCTP_FILE=$(find /project -maxdepth 1 -name "*.octp" -print -quit)\n\
if [ -z "$OCTP_FILE" ]; then\n\
    echo "Error: No .octp project file found in /project"\n\
    exit 1\n\
fi\n\
\n\
PROJECT_NAME=$(basename "$OCTP_FILE" .octp)\n\
echo "Building for Nintendo GameCube..."\n\
echo "Project: /project"\n\
echo "Project file: $OCTP_FILE"\n\
[ $DEBUG -eq 1 ] && echo "Debug mode: enabled"\n\
\n\
# Copy project to intermediate directory to protect original\n\
echo "Copying project to intermediate directory..."\n\
rm -rf /intermediate/*\n\
cp -r /project/* /intermediate/\n\
OCTP_FILE="/intermediate/$(basename $OCTP_FILE)"\n\
\n\
# Build using headless editor (cooks assets and compiles)\n\
cd /octave\n\
./OctaveEditor.elf -headless -project "$OCTP_FILE" -build GameCube embedded\n\
\n\
echo "Copying ROM to /game..."\n\
cp -r /intermediate/Packaged/GameCube/*.dol /game/\n\
chmod -R 777 /game\n\
\n\
echo "Done! GameCube build at /game/"\n\
' > /usr/local/bin/build-gcn && chmod +x /usr/local/bin/build-gcn

# build-wii: Build for Nintendo Wii with embedded assets
RUN echo '#!/bin/bash\n\
\n\
DEBUG=0\n\
for arg in "$@"; do\n\
    case $arg in\n\
        --debug|-d) DEBUG=1 ;;\n\
    esac\n\
done\n\
\n\
handle_error() {\n\
    echo ""\n\
    echo "=== BUILD FAILED ==="\n\
    if [ $DEBUG -eq 1 ]; then\n\
        echo "Debug mode: dropping to shell. Type exit to quit."\n\
        exec /bin/bash\n\
    fi\n\
    exit 1\n\
}\n\
trap handle_error ERR\n\
set -e\n\
\n\
if ! command -v powerpc-eabi-gcc &> /dev/null; then\n\
    echo "Error: devkitPPC not installed. Console builds unavailable."\n\
    exit 1\n\
fi\n\
\n\
if [ ! -d "/project" ] || [ -z "$(ls -A /project 2>/dev/null)" ]; then\n\
    echo "Error: No project mounted at /project"\n\
    echo "Usage: docker run -it --rm -v ./output:/game -v ./MyGame:/project octavegameengine build-wii [--debug]"\n\
    exit 1\n\
fi\n\
\n\
OCTP_FILE=$(find /project -maxdepth 1 -name "*.octp" -print -quit)\n\
if [ -z "$OCTP_FILE" ]; then\n\
    echo "Error: No .octp project file found in /project"\n\
    exit 1\n\
fi\n\
\n\
PROJECT_NAME=$(basename "$OCTP_FILE" .octp)\n\
echo "Building for Nintendo Wii..."\n\
echo "Project: /project"\n\
echo "Project file: $OCTP_FILE"\n\
[ $DEBUG -eq 1 ] && echo "Debug mode: enabled"\n\
\n\
# Copy project to intermediate directory to protect original\n\
echo "Copying project to intermediate directory..."\n\
rm -rf /intermediate/*\n\
cp -r /project/* /intermediate/\n\
OCTP_FILE="/intermediate/$(basename $OCTP_FILE)"\n\
\n\
# Build using headless editor (cooks assets and compiles)\n\
cd /octave\n\
./OctaveEditor.elf -headless -project "$OCTP_FILE" -build Wii embedded\n\
\n\
echo "Copying ROM to /game..."\n\
cp -r /intermediate/Packaged/Wii/*.dol /game/\n\
chmod -R 777 /game\n\
\n\
echo "Done! Wii build at /game/"\n\
' > /usr/local/bin/build-wii && chmod +x /usr/local/bin/build-wii

# Help command
RUN echo '#!/bin/bash\n\
echo "OctaveGameEngine Build Environment"\n\
echo "==================================="\n\
echo ""\n\
echo "The editor is pre-built and uses headless mode to cook assets."\n\
echo "Your project is copied to /intermediate before building to protect the original."\n\
echo ""\n\
echo "Commands:"\n\
echo "  build-editor  - Copy pre-built Linux Editor to output"\n\
echo "  build-linux   - Build Linux Game with embedded assets"\n\
echo "  build-3ds     - Build Nintendo 3DS with embedded assets"\n\
echo "  build-gcn     - Build Nintendo GameCube with embedded assets"\n\
echo "  build-wii     - Build Nintendo Wii with embedded assets"\n\
echo ""\n\
echo "Options:"\n\
echo "  --debug, -d   - Drop to shell on build failure (requires -it)"\n\
echo ""\n\
echo "Volumes:"\n\
echo "  /game         - Output directory (mount with -v ./output:/game)"\n\
echo "  /project      - Your game project with .octp file (mount with -v ./MyGame:/project)"\n\
echo "  /intermediate - (internal) Build working directory, protects /project from modification"\n\
echo ""\n\
echo "Examples:"\n\
echo "  docker run --rm -v ./output:/game octavegameengine build-editor"\n\
echo "  docker run --rm -v ./output:/game -v ./MyGame:/project octavegameengine build-linux"\n\
echo "  docker run -it --rm -v ./output:/game -v ./MyGame:/project octavegameengine build-3ds --debug"\n\
' > /usr/local/bin/help && chmod +x /usr/local/bin/help

VOLUME /game
VOLUME /project

ENTRYPOINT []
CMD ["help"]
