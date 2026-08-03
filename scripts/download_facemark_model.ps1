# Download the OpenCV LBF 68-point facemark model for landmark mode (Windows).
# Optional: without it the program runs in Haar fallback mode.
$ErrorActionPreference = "Stop"
Set-Location (Split-Path $PSScriptRoot -Parent)

New-Item -ItemType Directory -Force -Path models | Out-Null
$out = "models\lbfmodel.yaml"
$url = "https://raw.githubusercontent.com/kurnianggoro/GSOC2017/master/data/lbfmodel.yaml"

if (Test-Path $out) {
    Write-Host "Model already present at $out"
    exit 0
}

Write-Host "Downloading LBF facemark model to $out ..."
Invoke-WebRequest -Uri $url -OutFile $out
Write-Host "Done. Run with:  .\build\Release\dms.exe --model $out"
