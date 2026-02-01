<p align="center">
  <img src="OctaveLogo_128.png"/>
</p>

# Octave

A 3D Game Engine for GameCube, Wii, 3DS, Windows, Linux, and Android

[Tutorial Video](https://youtu.be/0CHLn0ie-DY)

[Lua Documentation](Documentation/Lua/README.md)

[Docker Documentation](Documentation/Docker/README.md)

[Editor Hotkeys](Documentation/Info/Editor.md)

**Check out the Releases page for precompiled standalone builds.**

---

Instructions for building from source below.

## Windows Compiling

1. Download and Install:
   - Visual Studio Community 2022 (with C++ support)
   - Vulkan SDK version 1.3.275.0 (During install select "Shader Toolchain Debug Symbols - 64 bit" and deselect all other options)
   - devkitPPC for GameCube/Wii development
   - devkitARM for 3DS development
   - instructions for installing the devkit tools can be found on the devkitpro wiki [here](https://devkitpro.org/wiki/Getting_Started)
2. Gamecube and Wii packaging currently only works on Linux or Docker. We recommend building Gamecube and Wii games on Windows with Docker see instructions below. But if you want to try to get them to package on Windows:
   - Open your Start menu and launch `devkitPro > MySys2`
   - `pacman-key --recv-keys C8A2759C315CFBC3429CC2E422B803BA8AA3D7CE --keyserver keyserver.ubuntu.com`
   - `pacman-key --lsign-key C8A2759C315CFBC3429CC2E422B803BA8AA3D7CE`
   - Put this entry in `/opt/devkitpro/pacman/etc/pacman.conf` above the `[dkp-libs]` entry: 
   ```
      [extremscorner-devkitpro]
      Server = https://packages.extremscorner.org/devkitpro/macos/$arch
   ```
   - `pacman -Syuu`
   - `pacman -S libogc2 libogc2-docs libogc2-examples`
   - `pacman -S libogc2-cmake`
      - Accept overwriting if asked.
   - Restart computer if  you've opened Visual Studio prior to installing `libogc2` to make sure the environment variables are found.
3. Build shaders by running compile.bat in `/Engine/Shaders/GLSL`.
4. Open Octave.sln.
5. Switch to the DebugEditor solution configuration.
6. Set the Standalone project as the Startup Project.
7. In the debug settings for Standalone, change the working directory to $(SolutionDir).
8. Build and run Standalone. This is the standalone level editor if you were making a game with Lua script only.

## Linux Compiling

### Installing Dependencies

1. For Debian/Ubuntu based distros:
   `sudo apt install g++ make libx11-dev libasound2-dev`
   For Arch-based distributions:
   `sudo pacman -S gcc make libx11 alsa-lib`
   Note: arch users may get a dependency error when attempting to install `alsa-lib`, in this case try to install `lib32-alsa-lib`.

2. Install Vulkan SDK version 1.3.275.0:

- Download the 1.3.275.0 tar file from <https://vulkan.lunarg.com/sdk/home#linux>
- Extract the tar file somewhere (e.g. ~/VulkanSDK/)
- Add these to your ~/.bashrc file (replace `~/VulkanSDK` with the directory where you extracted the files to). You may instead add these to a .sh file in your /etc/profiles.d directory to set up Vulkan for all users.

  ```
  export VULKAN_SDK=~/VulkanSDK/1.3.275.0/x86_64
  export PATH=$VULKAN_SDK/bin:$PATH
  export LD_LIBRARY_PATH=$VULKAN_SDK/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}
  export VK_LAYER_PATH=$VULKAN_SDK/share/vulkan/explicit_layer.d
  ```

- Close and reopen your terminal to apply the .bashrc (or run `source ~/.bashrc`)

1. Install devkitPro Pacman for GameCube/Wii/3DS development (Optional) (<https://devkitpro.org/wiki/devkitPro_pacman>)
   - wget <https://apt.devkitpro.org/install-devkitpro-pacman>
   - chmod +x ./install-devkitpro-pacman
   - sudo ./install-devkitpro-pacman
2. Install GameCube/Wii/3DS development libraries (Optional) (<https://devkitpro.org/wiki/Getting_Started>)
   - sudo dkp-pacman -S gamecube-dev
   - sudo dkp-pacman -S wii-dev
   - sudo dkp-pacman -S 3ds-dev
   - Restart computer
3. If you want to package Wii and Gamecube, Install `libogc2`
   - `sudo dkp-pacman-key --recv-keys C8A2759C315CFBC3429CC2E422B803BA8AA3D7CE --keyserver keyserver.ubuntu.com`
   - `sudo dkp-pacman-key --lsign-key C8A2759C315CFBC3429CC2E422B803BA8AA3D7CE`
   - Put this entry in `/opt/devkitpro/pacman/etc/pacman.conf` above the `[dkp-libs]` entry: 
   ```
      [extremscorner-devkitpro]
      Server = https://packages.extremscorner.org/devkitpro/macos/$arch
   ```
   - `sudo dkp-pacman -Syuu`
   - `sudo dkp-pacman -S libogc2 libogc2-docs libogc2-examples`
   - `sudo dkp-pacman -S libogc2-cmake`
      - Accept overwriting if asked.

4. cd Engine/Shaders/GLSL/ then run ./compile.sh.

### Compiling (Visual Studio Code)

1. Open the root octave folder in VsCode (where this README is located).
2. Install the C/C++ Extension Pack.
3. Run the Octave Editor config (click the Run and Debug tab on the left, then in the drop down where the green Play button is, select `Octave Editor`).

### Compiling (Terminal)

1. From the project's root directory (where this README is located) `cd Standalone`
2. Run `make -f Makefile_Linux_Editor`
3. Go back to the root directory `cd ..`
4. Run `Standalone/Build/Linux/OctaveEditor.elf` It's important that the working directory is the root directory where the Engine and Standalone folders are located.

## Packaging

1. When packaging for any platform on a Windows machine, you will likely need to install Msys2 so that linux commands can be executed. This comes packaged along with devkitPro libraries, so you if you install the devkitPro libraries, you shouldn't need to worry about this.
2. Packaging for Android requires installing Android Studio (Last tested with Android Studio 2022.2.1 Patch 2) with the following tools installed via the SDK Manager:
   - Android SDK Build Tools: 34.0.0
   - Android NDK (Side by side): 25.2.9519653
   - CMake: 3.22.1

## CMake Support

CMake support is currently a work-in-progress, and only Linux support has been implemented and tested. If you want to try building with CMake, here are some tips:

- Make sure you pull all submodules `git submodule update --init --recursive`
- Install pkg-config `sudo apt install pkg-config`(debian/ubuntu), `
- Install vorbis dev libraries `sudo apt install libvorbis-dev`

# Docker
Octave includes a Docker build system for reproducible builds across all supported platforms. You can also use the Docker system to build Octave itself from source without installing any dependencies on your host machine. You can get more information about using the Docker build system at [Documentation/Docker.md](Documentation/Info/Docker.md).

## Requirements
- Install Docker from <https://docs.docker.com/get-docker/>

## Build the Octave Docker Image
From your terminal, run:
```bash
# Clone the Octave repository if you haven't already, or to get the latest version
git clone https://github.com/mholtkamp/octave
# Move into the octave directory
cd octave
# Build the Docker image
./Docker/build.sh
```

## Packaging Games With Docker
To package your game using the Docker build system, run the following command from the root of your project directory (where your .octp file is located):
```bash
docker run --rm -v ./dist/3DS:/game -v .:/project octavegameengine build-3ds
```
This command mounts your project directory to `/project` in the Docker container, and tells the system to export your file to `./dist`. You should create the `dist` directory or whatever you want to export to beforehand or else the directory will be created by Docker and you will have to `sudo chmod -R 777 ./dist` to change permissions so you can access it.
 or do a `sudo rm -rf ./dist` to delete the directory.

### Available Docker Build Commands
- `build-linux` - Build a Linux `.elf` executable
- `build-gamecube` - Build a GameCube `.dol` file
- `build-wii` - Build a Wii `.dol` file
- `build-3ds` - Build a Nintendo `.3dsx` ROM


# Special Thanks

- Octave logo designed by overcookedchips.
