#!/usr/bin/env python3
"""
Integration tests for Ankh Vulkan renderer.

Tests verify the app starts without validation errors, crashes, or rendering failures.
Runs the app as a separate process with zero production code changes.

Usage:
    # Install pytest first (one time):
    pip install pytest
    
    # Run tests (from repo root or tests directory):
    pytest tests/test_integration.py -v
    
    # Or with explicit executable path:
    pytest tests/test_integration.py -v --ankh-path=/path/to/Ankh.exe
"""

import subprocess
import time
import os
import sys
from pathlib import Path
import pytest

# Timeout in seconds for app startup test
STARTUP_TIMEOUT_SEC = 5

# Timing tolerance in seconds for content rendering test
TIMING_TOLERANCE_SEC = 1.0

# Global to cache the executable path
_ankh_executable_path = None


def find_ankh_executable() -> Path:
    """Find the Ankh executable in common build locations."""
    # Get the tests directory (where this file is located)
    tests_dir = Path(__file__).parent.resolve()
    repo_root = tests_dir.parent
    
    # Also check current working directory
    cwd = Path.cwd()
    
    # Common build paths to check
    candidates = [
        # Relative to current directory (if running from build/src)
        cwd / "Ankh",
        cwd / "Ankh.exe",
        # Relative to repo root
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
            return candidate.resolve()
    
    return None


def get_ankh_path() -> str:
    """Get the path to the Ankh executable with auto-detection."""
    global _ankh_executable_path
    
    # Return cached path if available
    if _ankh_executable_path is not None:
        return str(_ankh_executable_path)
    
    # Check environment variable
    env_path = os.environ.get("ANKH_EXECUTABLE")
    if env_path:
        path = Path(env_path)
        if path.exists():
            _ankh_executable_path = path.resolve()
            return str(_ankh_executable_path)
    
    # Auto-detect
    found_path = find_ankh_executable()
    if found_path:
        _ankh_executable_path = found_path
        return str(_ankh_executable_path)
    
    raise FileNotFoundError(
        "Ankh executable not found. Please either:\n"
        "  1. Build the project first (cmake --build build)\n"
        "  2. Run pytest from the build/src directory\n"
        "  3. Set ANKH_EXECUTABLE environment variable\n"
        "  4. Use --ankh-path option: pytest --ankh-path=/path/to/Ankh.exe"
    )


def run_app_with_timeout(timeout_sec: int) -> tuple[int, str, float]:
    """
    Run the Ankh application for a specified duration and capture output.
    
    Args:
        timeout_sec: How long to wait before terminating the app.
    
    Returns:
        A tuple of (exit_code, stderr_output, elapsed_time_seconds):
        - exit_code 0: We successfully terminated the app after timeout (success case)
        - exit_code > 0: App exited on its own with this return code
        - exit_code < 0: App crashed with a signal (e.g., SIGSEGV = -11)
    """
    ankh_path = get_ankh_path()
    
    start_time = time.time()
    
    try:
        proc = subprocess.Popen(
            [ankh_path],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )
        
        try:
            # Wait for timeout
            stdout, stderr = proc.communicate(timeout=timeout_sec)
            elapsed = time.time() - start_time
            
            # Process exited on its own before timeout
            return proc.returncode, stderr.decode('utf-8', errors='replace'), elapsed
            
        except subprocess.TimeoutExpired:
            # Expected case - app is still running after timeout
            elapsed = time.time() - start_time
            
            # Gracefully terminate
            proc.terminate()
            
            try:
                stdout, stderr = proc.communicate(timeout=1)
            except subprocess.TimeoutExpired:
                # Force kill if not responding
                proc.kill()
                stdout, stderr = proc.communicate()
            
            # Successfully terminated by us - this is success
            return 0, stderr.decode('utf-8', errors='replace'), elapsed
            
    except FileNotFoundError:
        return -1, f"Executable not found: {ankh_path}", 0.0
    except Exception as e:
        return -1, str(e), 0.0


def has_validation_errors(stderr_output: str) -> bool:
    """
    Check if stderr contains Vulkan validation layer errors.
    
    Looks for specific patterns that indicate validation layer error messages,
    such as "validation error" or "Validation Error" appearing as a phrase.
    """
    lower_output = stderr_output.lower()
    # Check for common validation error patterns
    # Vulkan validation layers typically output "Validation Error:" or "validation error"
    return "validation error" in lower_output or "vk_layer" in lower_output and "error" in lower_output


class TestAppStartup:
    """Integration tests for application startup."""
    
    def test_starts_without_crash(self):
        """Verify the application starts without crashing."""
        exit_code, stderr, elapsed = run_app_with_timeout(STARTUP_TIMEOUT_SEC)
        
        # exit_code of 0 means we terminated it (success)
        # Negative values indicate crash signals
        assert exit_code >= 0, (
            f"Application crashed with exit code {exit_code}\n"
            f"Stderr: {stderr}"
        )
    
    def test_starts_without_validation_errors(self):
        """Verify the application starts without Vulkan validation errors."""
        exit_code, stderr, elapsed = run_app_with_timeout(STARTUP_TIMEOUT_SEC)
        
        assert not has_validation_errors(stderr), (
            f"Vulkan validation errors detected:\n{stderr}"
        )
    
    def test_content_is_rendered(self):
        """Verify the application runs for expected duration (rendering content)."""
        exit_code, stderr, elapsed = run_app_with_timeout(STARTUP_TIMEOUT_SEC)
        
        # If the app ran for close to the timeout, it was rendering
        min_expected_time = STARTUP_TIMEOUT_SEC - TIMING_TOLERANCE_SEC
        assert elapsed >= min_expected_time, (
            f"Application exited too early ({elapsed:.1f}s), may not have rendered content\n"
            f"Stderr: {stderr}"
        )


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
