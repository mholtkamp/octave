@echo off
REM =========================================================================
REM  build_installer_windows.bat
REM  Full build pipeline for Octave Engine Windows 64-bit installer.
REM
REM  This script performs the complete build process:
REM    1. Initialize git submodules
REM    2. Compile shaders
REM    3. Build Engine (ReleaseEditor x64)
REM    4. Stage distribution files
REM    5. Build Inno Setup installer
REM
REM  Prerequisites:
REM    - Visual Studio with MSBuild (or VS Developer Command Prompt)
REM    - Vulkan SDK installed (VULKAN_SDK environment variable set)
REM    - Python 3 on PATH
REM    - Inno Setup installed (ISCC.exe on PATH or at default location)
REM
REM  Usage: Installers\build_installer_windows.bat
REM =========================================================================

setlocal enabledelayedexpansion

REM Navigate to repo root
cd /d "%~dp0.."

echo ============================================
echo  Octave Engine - Windows 64-bit Full Build
echo ============================================
echo.

REM --- Check prerequisites ---
echo Checking prerequisites...

if not defined VULKAN_SDK (
    echo ERROR: VULKAN_SDK environment variable not set.
    echo        Install the Vulkan SDK and set VULKAN_SDK.
    exit /b 1
)
echo   [OK] VULKAN_SDK = %VULKAN_SDK%

where python >nul 2>&1
if errorlevel 1 (
    echo ERROR: Python not found on PATH.
    exit /b 1
)
echo   [OK] Python found

where msbuild >nul 2>&1
if errorlevel 1 (
    echo WARNING: MSBuild not found on PATH.
    echo          Run this script from a VS Developer Command Prompt,
    echo          or ensure MSBuild is in your PATH.
    echo.
)

echo.

REM --- Step 1: Initialize submodules ---
echo [1/5] Initializing git submodules...
git submodule init -- External/bullet3 External/doxygen-awesome-css External/zep Plugins/Blender/octave-gameengine-connect
if errorlevel 1 (
    echo ERROR: Submodule init failed.
    exit /b 1
)
git submodule update --recursive
if errorlevel 1 (
    echo ERROR: Submodule update failed.
    exit /b 1
)
echo   Submodules initialized.
echo.

REM --- Step 2: Compile shaders ---
echo [2/5] Compiling shaders...
pushd Engine\Shaders\GLSL
call compile.bat
if errorlevel 1 (
    popd
    echo ERROR: Shader compilation failed.
    exit /b 1
)
popd
echo   Shaders compiled.
echo.

REM --- Step 3: Build Engine ---
echo [3/5] Building Engine (ReleaseEditor x64)...
msbuild Octave.sln /p:Configuration=ReleaseEditor /p:Platform=x64 /m
if errorlevel 1 (
    echo ERROR: Engine build failed.
    exit /b 1
)
echo   Engine built successfully.
echo.

REM --- Step 4: Stage distribution files ---
echo [4/5] Staging distribution files...
python Installers\stage_distribution.py --platform windows --verbose
if errorlevel 1 (
    echo ERROR: Staging failed.
    exit /b 1
)
echo.

REM --- Step 5: Build installer ---
echo [5/5] Building installer...

set "ISCC="

REM Check PATH first
where iscc >nul 2>&1
if not errorlevel 1 (
    set "ISCC=iscc"
    goto :found_iscc
)

REM Check default install locations
if exist "C:\Program Files (x86)\Inno Setup 6\ISCC.exe" (
    set "ISCC=C:\Program Files (x86)\Inno Setup 6\ISCC.exe"
    goto :found_iscc
)
if exist "C:\Program Files\Inno Setup 6\ISCC.exe" (
    set "ISCC=C:\Program Files\Inno Setup 6\ISCC.exe"
    goto :found_iscc
)

echo ERROR: Inno Setup compiler (ISCC.exe) not found.
echo        Install Inno Setup 6 from https://jrsoftware.org/isinfo.php
echo        or add ISCC.exe to your PATH.
exit /b 1

:found_iscc
"%ISCC%" Installers\Windows\OctaveSetup.iss
if errorlevel 1 (
    echo ERROR: Inno Setup compilation failed.
    exit /b 1
)

echo.
echo ============================================
echo  BUILD COMPLETE!
echo ============================================
echo.
echo  Installer: dist\OctaveSetup-*.exe
echo.
echo ============================================
