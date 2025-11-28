cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed."
    exit 1
}

Set-Location tests
.venv/Scripts/activate.ps1
Set-Location ..

pytest tests/test_integration.py -v
