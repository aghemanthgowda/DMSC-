# Configure and build the Driver Monitoring System on Windows (PowerShell).
#
# Usage:
#   ./scripts/build.ps1                              # OpenCV found automatically or via $env:OpenCV_DIR
#   ./scripts/build.ps1 -OpenCVDir C:\opencv\build   # point at a prebuilt OpenCV
param(
    [string]$OpenCVDir = $env:OpenCV_DIR
)
$ErrorActionPreference = "Stop"
Set-Location (Split-Path $PSScriptRoot -Parent)

$args = @("-S", ".", "-B", "build")
if ($OpenCVDir) { $args += "-DOpenCV_DIR=$OpenCVDir" }
if ($env:VCPKG_ROOT) {
    $args += "-DCMAKE_TOOLCHAIN_FILE=$env:VCPKG_ROOT\scripts\buildsystems\vcpkg.cmake"
}

cmake @args
cmake --build build --config Release

Write-Host ""
Write-Host "Built: build\Release\dms.exe"
Write-Host "Run with:  .\build\Release\dms.exe"
Write-Host "  landmark mode:  .\build\Release\dms.exe --model models\lbfmodel.yaml"
