@echo off
REM Windows batch script to run integration tests with automatic setup
REM Usage: run_tests.bat [path_to_ankh_executable]

setlocal enabledelayedexpansion

echo === Ankh Integration Tests ===
echo.

REM Check if Python is installed
python --version >nul 2>&1
if errorlevel 1 (
    echo ERROR: Python is not installed or not in PATH.
    echo Please install Python 3.8+ from https://www.python.org/downloads/
    exit /b 1
)

REM Get script directory
set SCRIPT_DIR=%~dp0
set TESTS_DIR=%SCRIPT_DIR%

REM Create virtual environment if it doesn't exist
if not exist "%TESTS_DIR%.venv" (
    echo Creating Python virtual environment...
    python -m venv "%TESTS_DIR%.venv"
    if errorlevel 1 (
        echo ERROR: Failed to create virtual environment.
        exit /b 1
    )
)

REM Activate virtual environment
echo Activating virtual environment...
call "%TESTS_DIR%.venv\Scripts\activate.bat"

REM Install/upgrade pip and pytest
echo Installing dependencies...
python -m pip install --quiet --upgrade pip
python -m pip install --quiet pytest

REM Determine Ankh executable path
set ANKH_PATH=%1
if "%ANKH_PATH%"=="" (
    REM Try common build paths
    if exist "%SCRIPT_DIR%..\build\src\Ankh.exe" (
        set ANKH_PATH=%SCRIPT_DIR%..\build\src\Ankh.exe
    ) else if exist "%SCRIPT_DIR%..\build\src\Release\Ankh.exe" (
        set ANKH_PATH=%SCRIPT_DIR%..\build\src\Release\Ankh.exe
    ) else if exist "%SCRIPT_DIR%..\build\src\Debug\Ankh.exe" (
        set ANKH_PATH=%SCRIPT_DIR%..\build\src\Debug\Ankh.exe
    ) else if exist "%SCRIPT_DIR%..\out\build\x64-Debug\src\Ankh.exe" (
        set ANKH_PATH=%SCRIPT_DIR%..\out\build\x64-Debug\src\Ankh.exe
    ) else if exist "%SCRIPT_DIR%..\out\build\x64-Release\src\Ankh.exe" (
        set ANKH_PATH=%SCRIPT_DIR%..\out\build\x64-Release\src\Ankh.exe
    ) else (
        echo ERROR: Could not find Ankh.exe. Please build the project first or provide the path:
        echo   run_tests.bat path\to\Ankh.exe
        exit /b 1
    )
)

REM Verify executable exists
if not exist "%ANKH_PATH%" (
    echo ERROR: Ankh executable not found at: %ANKH_PATH%
    exit /b 1
)

echo.
echo Found Ankh executable: %ANKH_PATH%
echo.

REM Get directory containing Ankh.exe (for shader paths)
for %%I in ("%ANKH_PATH%") do set ANKH_DIR=%%~dpI

REM Run pytest from the Ankh directory (so shaders are found)
echo Running tests...
echo.
pushd "%ANKH_DIR%"
python -m pytest "%TESTS_DIR%test_integration.py" -v
set TEST_RESULT=%errorlevel%
popd

REM Deactivate virtual environment
call deactivate

echo.
if %TEST_RESULT% equ 0 (
    echo === All tests passed! ===
) else (
    echo === Some tests failed ===
)

exit /b %TEST_RESULT%
