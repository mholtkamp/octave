@echo off
REM =========================================================================
REM  build_installer_windows.bat
REM  Builds the Octave Engine Windows installer.
REM
REM  Prerequisites:
REM    - Python 3 on PATH
REM    - Inno Setup installed (ISCC.exe on PATH, or at default location)
REM    - Engine built: ReleaseEditor x64
REM
REM  Usage: Installers\build_installer_windows.bat
REM =========================================================================

setlocal

REM Navigate to repo root
cd /d "%~dp0.."

echo ============================================
echo  Octave Engine - Windows Installer Builder
echo ============================================
echo.

REM --- Step 1: Stage distribution files ---
echo [1/2] Staging distribution files...
python Installers\stage_distribution.py --platform windows --verbose
if errorlevel 1 (
    echo ERROR: Staging failed.
    exit /b 1
)
echo.

REM --- Step 2: Find Inno Setup compiler ---
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
echo [2/2] Building installer with Inno Setup...
"%ISCC%" Installers\Windows\OctaveSetup.iss
if errorlevel 1 (
    echo ERROR: Inno Setup compilation failed.
    exit /b 1
)

echo.
echo ============================================
echo  Installer built successfully!
echo  Output: dist\OctaveSetup-*.exe
echo ============================================
