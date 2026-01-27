# OctaveGameEngine - Universal Build Environment
#
# Usage:
#   Build image:
#     docker build -t octavegameengine -f Docker/Dockerfile .
#
#   Build Editor (no project needed):
#     docker run --rm -v ./output:/game octavegameengine build-editor
#
#   Build Game (pass your project directory):
#     docker run --rm -v ./output:/game -v ./MyGame:/project octavegameengine build-linux
#     docker run --rm -v ./output:/game -v ./MyGame:/project octavegameengine build-3ds
#     docker run --rm -v ./output:/game -v ./MyGame:/project octavegameengine build-gcn
#     docker run --rm -v ./output:/game -v ./MyGame:/project octavegameengine build-wii
#
#   Interactive:
#     docker run -it --rm -v ./output:/game -v ./MyGame:/project octavegameengine bash

FROM ubuntu:22.04

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
# This may fail on some systems - console builds will be unavailable
ENV DEVKITPRO=/opt/devkitpro
ENV DEVKITPPC=${DEVKITPRO}/devkitPPC
ENV DEVKITARM=${DEVKITPRO}/devkitARM
ENV PATH="${DEVKITPRO}/tools/bin:${DEVKITPPC}/bin:${DEVKITARM}/bin:${PATH}"

RUN apt-get update && \
    apt-get install -y gnupg && \
    wget -O /tmp/devkitpro-pub.gpg https://apt.devkitpro.org/devkitpro-pub.gpg && \
    gpg --dearmor -o /usr/share/keyrings/devkitpro-pub.gpg /tmp/devkitpro-pub.gpg && \
    echo "deb [signed-by=/usr/share/keyrings/devkitpro-pub.gpg] https://apt.devkitpro.org stable main" > /etc/apt/sources.list.d/devkitpro.list && \
    apt-get update && \
    apt-get install -y devkitpro-pacman && \
    dkp-pacman -Syu --noconfirm && \
    dkp-pacman -S --noconfirm gamecube-dev wii-dev 3ds-dev && \
    rm -rf /var/lib/apt/lists/* /tmp/* || \
    echo "WARNING: devkitPro installation failed - console builds will be unavailable"

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
# Build Commands
# ============================================================================

# build-editor: Build the Linux Editor
RUN echo '#!/bin/bash\n\
set -e\n\
\n\
# Check for project (optional but recommended)\n\
if [ ! -d "/project" ] || [ -z "$(ls -A /project 2>/dev/null)" ]; then\n\
    echo "Warning: No project mounted at /project"\n\
    echo "Usage: docker run --rm -v ./output:/game -v ./MyProject:/project octavegameengine build-editor"\n\
    echo "Building editor without project..."\n\
else\n\
    OCT_FILE=$(find /project -maxdepth 1 -name "*.oct" -print -quit)\n\
    echo "Project: /project"\n\
    [ -n "$OCT_FILE" ] && echo "Project file: $OCT_FILE"\n\
    echo "Copying project to /octave..."\n\
    cp -r /project/* /octave/\n\
fi\n\
\n\
rm -rf /octave/dist || true\n\
echo "Building Linux Editor..."\n\
cd /octave/Standalone\n\
make -j$(nproc) -f Makefile_Linux_Editor\n\
\n\
echo "Copying to /game..."\n\
cp /octave/Standalone/Build/Linux/OctaveEditor.elf /octave/\n\
cp -r /octave/* /game/\n\
\n\
echo "Done! Editor at /game/OctaveEditor.elf"\n\
' > /usr/local/bin/build-editor && chmod +x /usr/local/bin/build-editor

# build-linux: Build Linux Game
RUN echo '#!/bin/bash\n\
set -e\n\
\n\
if [ ! -d "/project" ] || [ -z "$(ls -A /project 2>/dev/null)" ]; then\n\
    echo "Error: No project mounted at /project"\n\
    echo "Usage: docker run --rm -v ./output:/game -v ./MyGame:/project octavegameengine build-linux"\n\
    exit 1\n\
fi\n\
\n\
OCT_FILE=$(find /project -maxdepth 1 -name "*.oct" -print -quit)\n\
echo "Building Linux Game..."\n\
echo "Project: /project"\n\
[ -n "$OCT_FILE" ] && echo "Project file: $OCT_FILE"\n\
\n\
cd /octave/Standalone\n\
make -j$(nproc) -f Makefile_Linux_Game\n\
\n\
echo "Copying to /game..."\n\
cp /octave/Standalone/Build/Linux/* /game/\n\
\n\
echo "Done! Output at /game/"\n\
' > /usr/local/bin/build-linux && chmod +x /usr/local/bin/build-linux

# build-3ds: Build for Nintendo 3DS
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
OCT_FILE=$(find /project -maxdepth 1 -name "*.oct" -print -quit)\n\
echo "Building for Nintendo 3DS..."\n\
echo "Project: /project"\n\
[ -n "$OCT_FILE" ] && echo "Project file: $OCT_FILE"\n\
\n\
cd /octave/Standalone\n\
make -j$(nproc) -f Makefile_3DS\n\
\n\
echo "Copying to /game..."\n\
cp /octave/Standalone/Build/3DS/* /game/\n\
\n\
echo "Done! 3DS build at /game/"\n\
' > /usr/local/bin/build-3ds && chmod +x /usr/local/bin/build-3ds

# build-gcn: Build for Nintendo GameCube
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
OCT_FILE=$(find /project -maxdepth 1 -name "*.oct" -print -quit)\n\
echo "Building for Nintendo GameCube..."\n\
echo "Project: /project"\n\
[ -n "$OCT_FILE" ] && echo "Project file: $OCT_FILE"\n\
\n\
cd /octave/Standalone\n\
make -j$(nproc) -f Makefile_GCN\n\
\n\
echo "Copying to /game..."\n\
cp /octave/Standalone/Build/GCN/* /game/\n\
\n\
echo "Done! GameCube build at /game/"\n\
' > /usr/local/bin/build-gcn && chmod +x /usr/local/bin/build-gcn

# build-wii: Build for Nintendo Wii
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
OCT_FILE=$(find /project -maxdepth 1 -name "*.oct" -print -quit)\n\
echo "Building for Nintendo Wii..."\n\
echo "Project: /project"\n\
[ -n "$OCT_FILE" ] && echo "Project file: $OCT_FILE"\n\
\n\
cd /octave/Standalone\n\
make -j$(nproc) -f Makefile_Wii\n\
\n\
echo "Copying to /game..."\n\
cp /octave/Standalone/Build/Wii/* /game/\n\
\n\
echo "Done! Wii build at /game/"\n\
' > /usr/local/bin/build-wii && chmod +x /usr/local/bin/build-wii

# Help command
RUN echo '#!/bin/bash\n\
echo "OctaveGameEngine Build Environment"\n\
echo "==================================="\n\
echo ""\n\
echo "Commands:"\n\
echo "  build-editor  - Build Linux Editor (project optional)"\n\
echo "  build-linux   - Build Linux Game (project required)"\n\
echo "  build-3ds     - Build for Nintendo 3DS (project required)"\n\
echo "  build-gcn     - Build for Nintendo GameCube (project required)"\n\
echo "  build-wii     - Build for Nintendo Wii (project required)"\n\
echo ""\n\
echo "Volumes:"\n\
echo "  /game     - Output directory (mount with -v ./output:/game)"\n\
echo "  /project  - Your game project (mount with -v ./MyGame:/project)"\n\
echo ""\n\
echo "Examples:"\n\
echo "  docker run --rm -v ./output:/game octavegameengine build-editor"\n\
echo "  docker run --rm -v ./output:/game -v ./MyGame:/project octavegameengine build-linux"\n\
' > /usr/local/bin/help && chmod +x /usr/local/bin/help

VOLUME /game
VOLUME /project

ENTRYPOINT []
CMD ["help"]
