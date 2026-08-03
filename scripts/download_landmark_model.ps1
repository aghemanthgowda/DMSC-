# Download the dlib 68-point facial-landmark model for accurate eye-closure,
# yawn and head-pose detection (Windows). The file is served UNCOMPRESSED, so
# no extra decompression tool is needed.
$ErrorActionPreference = "Stop"
Set-Location (Split-Path $PSScriptRoot -Parent)

New-Item -ItemType Directory -Force -Path models | Out-Null
$out = "models\shape_predictor_68_face_landmarks.dat"
$url = "https://raw.githubusercontent.com/italojs/facial-landmarks-recognition/master/shape_predictor_68_face_landmarks.dat"

if (Test-Path $out) {
    Write-Host "Model already present at $out"
    exit 0
}

Write-Host "Downloading dlib 68-point model (~96 MB) to $out ..."
Invoke-WebRequest -Uri $url -OutFile $out
Write-Host "Done. It will be picked up automatically by dms.exe."
