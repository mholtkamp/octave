# octave
Game Engine

## Windows Setup
1. Download and Install:
	- Visual Studio Community 2017 (with C++ support)
	- Vulkan SDK
2. Build shaders by running compile.bat in Engine/Shaders/GLSL
3. Set Rocket as Startup Project in VS2017
4. Set Rocket's Debug working directory to $(SolutionDir)
5. Launch Rocket in DebugEditor to open editor
6. Ctrl+B to build data in editor
7. Launch Rocket in Debug to run game
8. If you want to package the Windows project with Ctrl+B in the Editor, add devenv to the PATH. My devenv for VS 2017 community was located here: "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\IDE"