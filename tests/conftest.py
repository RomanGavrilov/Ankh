"""
Pytest configuration for Ankh integration tests.
"""
import os
import pytest


def pytest_addoption(parser):
    """Add custom command line option for Ankh executable path."""
    parser.addoption(
        "--ankh-path",
        action="store",
        default=None,
        help="Path to Ankh executable"
    )


def pytest_configure(config):
    """Set environment variable from command line option."""
    ankh_path = config.getoption("--ankh-path")
    if ankh_path:
        os.environ["ANKH_EXECUTABLE"] = ankh_path

