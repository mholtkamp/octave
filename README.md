# octave
A Simple 3D Game Engine for GameCube, Wii, 3DS, Windows, and Linux

## Windows Setup
1. Download and Install:
	- Visual Studio Community 2017 (with C++ support)
	- Vulkan SDK
    - devkitPPC for GameCube/Wii development (Optional)
    - devkitARM for 3DS development (Optional)
2. Build shaders by running compile.bat in Engine/Shaders/GLSL
3. Open Octave.sln
4. Switch to the DebugEditor solution configuration
5. Set the Standalone project as the Startup Project
6. In the debug settings for Standalone, change the working directory to $(SolutionDir)
7. Build and run Standalone. This is the standalone level edtior if you were making a game with Lua script only.

## Linux Setup
1. sudo apt-get install g++
2. sudo apt-get install make
3. sudo apt-get install libx11-dev
4. sudo apt-get install libasound2-dev
5. Install Vulkan SDK
    - wget -qO - http://packages.lunarg.com/lunarg-signing-key-pub.asc | sudo apt-key add -
    - sudo wget -qO /etc/apt/sources.list.d/lunarg-vulkan-focal.list http://packages.lunarg.com/vulkan/lunarg-vulkan-focal.list
    - sudo apt update
    - sudo apt install vulkan-sdk
6. Install devkitPro Pacman for GameCube/Wii/3DS development (Optional) (https://devkitpro.org/wiki/devkitPro_pacman)
    - wget https://apt.devkitpro.org/install-devkitpro-pacman
    - chmod +x ./install-devkitpro-pacman
    - sudo ./install-devkitpro-pacman
7. Install GameCube/Wii/3DS development libraries (Optional) (https://devkitpro.org/wiki/Getting_Started)
    - sudo dkp-pacman -S gamecube-dev
    - sudo dkp-pacman -S wii-dev
    - sudo dkp-pacman -S 3ds-dev
8. Run Engine/Shaders/GLSL/compile.sh

### Linux Setup (Visual Studio Code)
9. Open the root folder in VsCode.
10. Run the Linux Editor config.

### Linux Setup (Terminal)
9. From the root directory `cd Standalone` 
10. Run `make -f Makefile_Linux_Editor`
11. Go back to the root directory `cd ..`
12. Run `Standalone/Build/Linux/OctaveEditor.out` It's important that the working directory is the root directory where the Engine and Standalone folders are located.
