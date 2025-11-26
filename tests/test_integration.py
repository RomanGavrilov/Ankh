#!/usr/bin/env python3
"""
Integration tests for Ankh Vulkan renderer.

Tests verify the app starts without validation errors, crashes, or rendering failures.
Runs the app as a separate process with zero production code changes.
"""

import subprocess
import time
import os
import pytest

# Path to Ankh executable (relative to build/src directory where tests run)
ANKH_PATH = "./Ankh"

# Timeout in seconds for app startup test
STARTUP_TIMEOUT_SEC = 5


def get_ankh_path():
    """Get the path to the Ankh executable."""
    if os.path.exists(ANKH_PATH):
        return ANKH_PATH
    raise FileNotFoundError(f"Ankh executable not found at {ANKH_PATH}")


def run_app_with_timeout(timeout_sec: int) -> tuple[int, str, float]:
    """
    Run the Ankh application for a specified duration and capture output.
    
    Returns:
        tuple of (exit_code, stderr_output, elapsed_time_seconds)
        exit_code: 0 if we terminated it, negative if crashed with signal, positive if exited
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
    """Check if stderr contains Vulkan validation errors (case-insensitive)."""
    lower_output = stderr_output.lower()
    return "validation" in lower_output and "error" in lower_output


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
        assert elapsed >= STARTUP_TIMEOUT_SEC - 1, (
            f"Application exited too early ({elapsed:.1f}s), may not have rendered content\n"
            f"Stderr: {stderr}"
        )


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
