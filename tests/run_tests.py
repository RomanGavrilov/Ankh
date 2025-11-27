#!/usr/bin/env python3
"""
Automated test runner for Ankh.

This script builds the Ankh executable and runs the integration tests.
It handles CMake configuration, building, and running pytest automatically.

Usage:
    python tests/run_tests.py
    python tests/run_tests.py --build-type Release
    python tests/run_tests.py --verbose
"""

import argparse
import os
import subprocess
import sys
from pathlib import Path


def get_repo_root() -> Path:
    """Get the repository root directory."""
    return Path(__file__).parent.parent.resolve()


def get_build_dir(repo_root: Path) -> Path:
    """Get the default build directory."""
    return repo_root / "build"


def run_command(cmd: list[str], cwd: Path, description: str, check: bool = True) -> int:
    """Run a command and print its output."""
    print(f"\n{'='*60}")
    print(f"  {description}")
    print(f"{'='*60}")
    print(f"Running: {' '.join(cmd)}")
    print(f"In directory: {cwd}")
    print()

    result = subprocess.run(cmd, cwd=cwd)

    if check and result.returncode != 0:
        print(f"\nError: Command failed with exit code {result.returncode}")
        sys.exit(result.returncode)

    return result.returncode


def configure_cmake(repo_root: Path, build_dir: Path, build_type: str) -> None:
    """Configure CMake if needed."""
    cmake_cache = build_dir / "CMakeCache.txt"

    if cmake_cache.exists():
        print(f"\nCMake already configured (found {cmake_cache})")
        return

    print(f"\nConfiguring CMake...")
    build_dir.mkdir(parents=True, exist_ok=True)

    cmd = [
        "cmake",
        str(repo_root),
        f"-DCMAKE_BUILD_TYPE={build_type}",
    ]

    run_command(cmd, build_dir, f"Configuring CMake ({build_type})")


def build_ankh(build_dir: Path, build_type: str) -> None:
    """Build the Ankh executable."""
    cmd = [
        "cmake",
        "--build",
        str(build_dir),
        "--target",
        "Ankh",
        "--config",
        build_type,
    ]

    run_command(cmd, build_dir, "Building Ankh")


def run_pytest(repo_root: Path, build_dir: Path, verbose: bool) -> int:
    """Run pytest integration tests."""
    tests_dir = repo_root / "tests"
    test_file = tests_dir / "test_integration.py"

    # Build pytest command
    cmd = [sys.executable, "-m", "pytest", str(test_file)]

    if verbose:
        cmd.append("-v")

    # The tests auto-detect the executable in common build locations
    return run_command(cmd, repo_root, "Running integration tests", check=False)


def main() -> int:
    """Main entry point."""
    parser = argparse.ArgumentParser(
        description="Build Ankh and run integration tests"
    )
    parser.add_argument(
        "--build-type",
        choices=["Debug", "Release", "RelWithDebInfo", "MinSizeRel"],
        default="Debug",
        help="CMake build type (default: Debug)",
    )
    parser.add_argument(
        "--verbose", "-v",
        action="store_true",
        help="Verbose output",
    )
    parser.add_argument(
        "--skip-build",
        action="store_true",
        help="Skip the build step (use existing build)",
    )
    parser.add_argument(
        "--build-dir",
        type=Path,
        help="Custom build directory (default: <repo>/build)",
    )

    args = parser.parse_args()

    repo_root = get_repo_root()
    build_dir = args.build_dir if args.build_dir else get_build_dir(repo_root)

    print(f"Repository root: {repo_root}")
    print(f"Build directory: {build_dir}")
    print(f"Build type: {args.build_type}")

    if not args.skip_build:
        # Step 1: Configure CMake
        configure_cmake(repo_root, build_dir, args.build_type)

        # Step 2: Build Ankh
        build_ankh(build_dir, args.build_type)
    else:
        print("\nSkipping build step (--skip-build)")

    # Step 3: Run pytest
    exit_code = run_pytest(repo_root, build_dir, args.verbose)

    if exit_code == 0:
        print("\n" + "="*60)
        print("  All tests passed!")
        print("="*60)
    else:
        print("\n" + "="*60)
        print(f"  Tests failed with exit code {exit_code}")
        print("="*60)

    return exit_code


if __name__ == "__main__":
    sys.exit(main())
