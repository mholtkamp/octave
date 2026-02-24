#!/usr/bin/env python3
"""
stage_distribution.py - Assembles distributable files for Octave Engine.

Copies only the files needed for distribution from the dev tree into a
clean output directory. This is the shared foundation for both Windows
and Linux installers.

Usage:
    python stage_distribution.py --platform windows|linux [--output-dir dist/Editor] [--verbose]
"""

import argparse
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

# ---------------------------------------------------------------------------
# Configuration
# ---------------------------------------------------------------------------

# Directories to exclude everywhere
EXCLUDED_DIRS = {
    ".git", ".vs", ".claude", ".dev", ".llm",
    "__pycache__", "node_modules", "Build", "doxygen_xml",
}

# File extensions to exclude everywhere
EXCLUDED_EXTENSIONS = {
    ".pdb", ".ilk", ".exp", ".lib", ".obj", ".o", ".a",
    ".user",
    ".log", ".tmp", ".bak",
}


def log(msg, verbose=False):
    if verbose:
        print(f"  {msg}")


def extract_version(engine_root):
    """Extract OCTAVE_VERSION from Constants.h."""
    constants_path = engine_root / "Engine" / "Source" / "Engine" / "Constants.h"
    if not constants_path.exists():
        print(f"WARNING: {constants_path} not found, defaulting to version 0")
        return "0"
    text = constants_path.read_text(encoding="utf-8", errors="replace")
    match = re.search(r"#define\s+OCTAVE_VERSION\s+(\d+)", text)
    if match:
        return match.group(1)
    print("WARNING: OCTAVE_VERSION not found in Constants.h, defaulting to 0")
    return "0"


def should_exclude_dir(name):
    return name in EXCLUDED_DIRS


def should_exclude_file(path):
    return path.suffix.lower() in EXCLUDED_EXTENSIONS


def copy_tree(src, dst, verbose=False, file_filter=None):
    """Recursively copy a directory tree, respecting exclusion rules.

    file_filter: optional callable(Path) -> bool. If provided, only files
                 for which it returns True are copied.
    """
    src = Path(src)
    dst = Path(dst)
    if not src.is_dir():
        log(f"SKIP (not found): {src}", verbose)
        return 0
    count = 0
    for item in sorted(src.iterdir()):
        if item.is_dir():
            if should_exclude_dir(item.name):
                continue
            count += copy_tree(item, dst / item.name, verbose, file_filter)
        elif item.is_file():
            if should_exclude_file(item):
                continue
            if file_filter and not file_filter(item):
                continue
            dst.mkdir(parents=True, exist_ok=True)
            shutil.copy2(item, dst / item.name)
            count += 1
    return count


def copy_file(src, dst, verbose=False):
    """Copy a single file, creating parent directories as needed."""
    src = Path(src)
    dst = Path(dst)
    if not src.exists():
        log(f"SKIP (not found): {src}", verbose)
        return False
    dst.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(src, dst)
    return True



def run_generators(engine_root, verbose=False):
    """Run code generators to ensure Generated/ is fresh."""
    tools_dir = engine_root / "Tools"

    generators = [
        ("generate_lua_stubs.py", "Generating Lua IntelliSense stubs..."),
        ("generate_ui_xsd.py", "Generating UI XSD schema..."),
    ]

    for script_name, msg in generators:
        script = tools_dir / script_name
        if not script.exists():
            print(f"  WARNING: {script} not found, skipping")
            continue
        print(f"  {msg}")
        args = [sys.executable, str(script)]
        if verbose:
            args.append("--verbose")
        result = subprocess.run(args, cwd=str(engine_root), capture_output=not verbose)
        if result.returncode != 0:
            print(f"  WARNING: {script_name} returned exit code {result.returncode}")
            if result.stderr:
                print(f"  stderr: {result.stderr.decode(errors='replace')}")


def stage(platform, output_dir, engine_root, verbose=False):
    """Main staging logic."""
    dist = Path(output_dir)

    # Clean previous staging
    if dist.exists():
        print(f"Cleaning previous staging directory: {dist}")
        shutil.rmtree(dist)

    dist.mkdir(parents=True, exist_ok=True)

    version = extract_version(engine_root)
    print(f"Octave Engine version: {version}")
    print(f"Platform: {platform}")
    print(f"Output: {dist.resolve()}")
    print()

    # Run generators first
    print("Running generators...")
    run_generators(engine_root, verbose)
    print()

    # --- Binary ---
    print("Staging binary...")
    if platform == "windows":
        binary_src = engine_root / "Standalone" / "Build" / "Windows" / "x64" / "ReleaseEditor" / "Octave.exe"
        binary_dst = dist / "Octave.exe"
    else:
        # Linux binary - Makefile produces OctaveEditor.elf, we install as OctaveEditor
        binary_src = engine_root / "Standalone" / "Build" / "Linux" / "OctaveEditor.elf"
        if not binary_src.exists():
            binary_src = engine_root / "Build" / "Linux" / "OctaveEditor.elf"
        if not binary_src.exists():
            binary_src = engine_root / "OctaveEditor.elf"
        if not binary_src.exists():
            # Also try without .elf extension
            binary_src = engine_root / "Standalone" / "Build" / "Linux" / "OctaveEditor"
        binary_dst = dist / "OctaveEditor"

    if copy_file(binary_src, binary_dst, verbose):
        log(f"Binary: {binary_src}", verbose)
    else:
        print(f"  WARNING: Binary not found at {binary_src}")
        print("           Build the engine first before staging.")

    # --- Engine Assets ---
    print("Staging Engine/Assets...")
    n = copy_tree(engine_root / "Engine" / "Assets", dist / "Engine" / "Assets", verbose)
    log(f"  {n} files", verbose)

    # --- Shaders ---
    print("Staging Engine/Shaders...")
    n = copy_tree(engine_root / "Engine" / "Shaders", dist / "Engine" / "Shaders", verbose)
    log(f"  {n} files", verbose)

    # --- Scripts ---
    print("Staging Engine/Scripts...")
    n = copy_tree(engine_root / "Engine" / "Scripts", dist / "Engine" / "Scripts", verbose)
    log(f"  {n} files", verbose)

    # --- Generated ---
    print("Staging Engine/Generated...")
    n = copy_tree(engine_root / "Engine" / "Generated", dist / "Engine" / "Generated", verbose)
    log(f"  {n} files", verbose)

    # --- Engine Source (full source for builds and SDK) ---
    print("Staging Engine/Source...")
    n = copy_tree(
        engine_root / "Engine" / "Source",
        dist / "Engine" / "Source",
        verbose,
    )
    log(f"  {n} files", verbose)

    # --- Engine project/build files ---
    print("Staging Engine project files...")
    for fname in ["Engine.vcxproj", "Engine.vcxproj.filters", "CMakeLists.txt",
                   "Makefile_3DS", "Makefile_GCN", "Makefile_Wii", "Makefile_Linux"]:
        copy_file(engine_root / "Engine" / fname, dist / "Engine" / fname, verbose)

    # --- External (full directory for builds) ---
    print("Staging External/...")
    n = copy_tree(engine_root / "External", dist / "External", verbose)
    log(f"  {n} files", verbose)

    # --- Template ---
    print("Staging Template/...")
    n = copy_tree(engine_root / "Template", dist / "Template", verbose)
    log(f"  {n} files", verbose)

    # --- Standalone (full directory for builds and C++ project creation) ---
    print("Staging Standalone/...")
    n = copy_tree(engine_root / "Standalone", dist / "Standalone", verbose)
    log(f"  {n} files", verbose)

    # --- Prebuilt game executable (for standalone packaging without recompile) ---
    print("Staging prebuilt game executable...")
    if platform == "windows":
        game_exe = engine_root / "Standalone" / "Build" / "Windows" / "x64" / "Release" / "Octave.exe"
        game_dst = dist / "Standalone" / "Build" / "Windows" / "x64" / "Release" / "Octave.exe"
        if copy_file(game_exe, game_dst, verbose):
            log(f"Game exe: {game_exe}", verbose)
        else:
            print("  WARNING: Prebuilt game exe not found (builds will require compilation)")
    else:
        game_exe = engine_root / "Standalone" / "Build" / "Linux" / "Octave.elf"
        game_dst = dist / "Standalone" / "Build" / "Linux" / "Octave.elf"
        if copy_file(game_exe, game_dst, verbose):
            log(f"Game exe: {game_exe}", verbose)
        else:
            print("  WARNING: Prebuilt game exe not found (builds will require compilation)")

    # --- Octave.sln (needed for C++ project creation) ---
    print("Staging Octave.sln...")
    copy_file(engine_root / "Octave.sln", dist / "Octave.sln", verbose)

    # --- .vscode (needed for C++ project creation) ---
    print("Staging .vscode/...")
    n = copy_tree(engine_root / ".vscode", dist / ".vscode", verbose)
    log(f"  {n} files", verbose)

    # --- Tools ---
    print("Staging Tools/...")
    n = copy_tree(engine_root / "Tools", dist / "Tools", verbose)
    log(f"  {n} files", verbose)

    # --- Documentation ---
    print("Staging Documentation/...")
    n = copy_tree(engine_root / "Documentation", dist / "Documentation", verbose)
    log(f"  {n} files", verbose)

    # --- Root files ---
    print("Staging root files...")
    for fname in ["LICENSE", "OctaveLogo_128.png", "OctaveLogo_256.png"]:
        copy_file(engine_root / fname, dist / fname, verbose)

    # --- Create Engine/Saves directory (writable at runtime) ---
    (dist / "Engine" / "Saves").mkdir(parents=True, exist_ok=True)

    # --- Write version file ---
    # INI format so Inno Setup's ReadIni() can parse it
    (dist / "version.txt").write_text(
        f"[Octave]\nVersion={version}\n", encoding="utf-8"
    )

    print()
    print(f"Staging complete. Version {version} staged to: {dist.resolve()}")
    return version


def main():
    parser = argparse.ArgumentParser(
        description="Stage Octave Engine files for distribution."
    )
    parser.add_argument(
        "--platform",
        required=True,
        choices=["windows", "linux"],
        help="Target platform",
    )
    parser.add_argument(
        "--output-dir",
        default="dist/Editor",
        help="Output directory (default: dist/Editor)",
    )
    parser.add_argument(
        "--verbose", "-v",
        action="store_true",
        help="Verbose output",
    )

    args = parser.parse_args()

    # Engine root is the repo root (parent of Installers/)
    engine_root = Path(__file__).resolve().parent.parent

    if not (engine_root / "Engine").is_dir():
        print(f"ERROR: Engine/ not found at {engine_root}")
        print("       Run this script from the Installers/ directory or repo root.")
        sys.exit(1)

    # Resolve output dir relative to engine root
    output_dir = Path(args.output_dir)
    if not output_dir.is_absolute():
        output_dir = engine_root / output_dir

    stage(args.platform, output_dir, engine_root, args.verbose)


if __name__ == "__main__":
    main()
