#!/usr/bin/env python3
"""
Cross-platform test runner for Ankh integration tests.

This script:
1. Creates a Python virtual environment (if needed)
2. Installs pytest
3. Locates the Ankh executable
4. Runs the integration tests

Usage:
    python run_tests.py [path_to_ankh_executable]
    
On Windows:
    python run_tests.py
    
On Linux/macOS:
    python3 run_tests.py
"""

import os
import sys
import subprocess
import venv
from pathlib import Path


def find_ankh_executable(script_dir: Path) -> Path:
    """Find the Ankh executable in common build locations."""
    # Common build paths to check (relative to repo root)
    repo_root = script_dir.parent
    
    candidates = [
        # CMake standard builds
        repo_root / "build" / "src" / "Ankh",
        repo_root / "build" / "src" / "Ankh.exe",
        repo_root / "build" / "src" / "Release" / "Ankh.exe",
        repo_root / "build" / "src" / "Debug" / "Ankh.exe",
        # Visual Studio / CMake presets
        repo_root / "out" / "build" / "x64-Debug" / "src" / "Ankh.exe",
        repo_root / "out" / "build" / "x64-Release" / "src" / "Ankh.exe",
        # Ninja builds
        repo_root / "build" / "Ankh",
        repo_root / "build" / "Ankh.exe",
    ]
    
    for candidate in candidates:
        if candidate.exists():
            return candidate
    
    return None


def create_venv(venv_path: Path) -> None:
    """Create a virtual environment."""
    print(f"Creating virtual environment at {venv_path}...")
    venv.create(venv_path, with_pip=True)


def get_venv_python(venv_path: Path) -> Path:
    """Get the Python executable path for the virtual environment."""
    if sys.platform == "win32":
        return venv_path / "Scripts" / "python.exe"
    return venv_path / "bin" / "python"


def run_command(cmd: list, cwd: Path = None) -> int:
    """Run a command and return the exit code."""
    result = subprocess.run(cmd, cwd=cwd)
    return result.returncode


def main():
    script_dir = Path(__file__).parent.resolve()
    venv_path = script_dir / ".venv"
    
    print("=" * 50)
    print("Ankh Integration Tests")
    print("=" * 50)
    print()
    
    # Check for custom executable path
    ankh_path = None
    if len(sys.argv) > 1:
        ankh_path = Path(sys.argv[1])
        if not ankh_path.exists():
            print(f"ERROR: Specified executable not found: {ankh_path}")
            return 1
    else:
        # Auto-detect
        ankh_path = find_ankh_executable(script_dir)
        if not ankh_path:
            print("ERROR: Could not find Ankh executable.")
            print("Please build the project first, or provide the path:")
            print(f"  python {Path(__file__).name} path/to/Ankh.exe")
            return 1
    
    print(f"Found Ankh executable: {ankh_path}")
    print()
    
    # Create venv if needed
    if not venv_path.exists():
        create_venv(venv_path)
    
    venv_python = get_venv_python(venv_path)
    
    if not venv_python.exists():
        print(f"ERROR: Virtual environment Python not found at {venv_python}")
        return 1
    
    # Install pytest
    print("Installing dependencies...")
    result = run_command([
        str(venv_python), "-m", "pip", "install", "--quiet", "--upgrade", "pip"
    ])
    if result != 0:
        print("ERROR: Failed to upgrade pip")
        return 1
    
    result = run_command([
        str(venv_python), "-m", "pip", "install", "--quiet", "pytest"
    ])
    if result != 0:
        print("ERROR: Failed to install pytest")
        return 1
    
    # Run tests from the Ankh executable directory (for shader paths)
    ankh_dir = ankh_path.parent
    test_file = script_dir / "test_integration.py"
    
    print()
    print("Running tests...")
    print()
    
    result = run_command(
        [str(venv_python), "-m", "pytest", str(test_file), "-v"],
        cwd=ankh_dir
    )
    
    print()
    if result == 0:
        print("=" * 50)
        print("All tests passed!")
        print("=" * 50)
    else:
        print("=" * 50)
        print("Some tests failed")
        print("=" * 50)
    
    return result


if __name__ == "__main__":
    sys.exit(main())
