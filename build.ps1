# Build script for ddogreen using CMake on Windows
# PowerShell version of build.sh

# Set error action preference to stop on errors
$ErrorActionPreference = "Stop"

Write-Host "Building ddogreen - Intelligent Green Power Management" -ForegroundColor Green

# Check if CMake is available
try {
    $cmakeVersion = cmake --version 2>$null
    Write-Host "Found CMake: $($cmakeVersion[0])" -ForegroundColor Blue
} catch {
    Write-Host "CMake not found. Please install CMake:" -ForegroundColor Red
    Write-Host "  Download from: https://cmake.org/download/" -ForegroundColor Yellow
    Write-Host "  Or via chocolatey: choco install cmake" -ForegroundColor Yellow
    Write-Host "  Or via winget: winget install Kitware.CMake" -ForegroundColor Yellow
    exit 1
}

# Check if Visual Studio Build Tools or MSVC is available
$vcvarsFound = $false
$generators = @(
    "Visual Studio 17 2022",
    "Visual Studio 16 2019", 
    "Visual Studio 15 2017",
    "MinGW Makefiles"
)

# Try to find MSVC
try {
    $vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $vsWhere) {
        $vsInstall = & $vsWhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
        if ($vsInstall) {
            $vcvarsFound = $true
            Write-Host "Found Visual Studio at: $vsInstall" -ForegroundColor Blue
        }
    }
} catch {
    # Continue to check other options
}

# Check for MinGW if MSVC not found
if (-not $vcvarsFound) {
    try {
        $mingwVersion = g++ --version 2>$null
        if ($mingwVersion) {
            Write-Host "Found MinGW: $($mingwVersion[0])" -ForegroundColor Blue
            $vcvarsFound = $true
        }
    } catch {
        Write-Host "No C++ compiler found. Please install one of:" -ForegroundColor Red
        Write-Host "  Visual Studio 2019/2022 with C++ workload" -ForegroundColor Yellow
        Write-Host "  Visual Studio Build Tools" -ForegroundColor Yellow
        Write-Host "  MinGW-w64 (via MSYS2, chocolatey, or winget)" -ForegroundColor Yellow
        exit 1
    }
}

# Create and enter build directory
if (Test-Path "build") {
    Write-Host "Cleaning previous build directory..." -ForegroundColor Yellow
    Remove-Item "build" -Recurse -Force
}
New-Item -ItemType Directory -Name "build" | Out-Null
Set-Location "build"

# Configure with CMake
Write-Host "Configuring with CMake..." -ForegroundColor Green

# Try different generators until one works
$configSuccess = $false
foreach ($generator in $generators) {
    try {
        Write-Host "Trying generator: $generator" -ForegroundColor Blue
        cmake .. -G "$generator"
        $configSuccess = $true
        Write-Host "Configuration successful with $generator" -ForegroundColor Green
        break
    } catch {
        Write-Host "Generator $generator failed, trying next..." -ForegroundColor Yellow
        continue
    }
}

if (-not $configSuccess) {
    Write-Host "All generators failed. Please check your development environment." -ForegroundColor Red
    exit 1
}

# Build
Write-Host "Building..." -ForegroundColor Green

# Determine number of cores for parallel build
$cores = (Get-CimInstance -ClassName Win32_ComputerSystem).NumberOfLogicalProcessors

try {
    cmake --build . --config Release --parallel $cores
    if ($LASTEXITCODE -ne 0) {
        throw "Build failed with exit code $LASTEXITCODE"
    }
} catch {
    Write-Host "Parallel build failed. Trying without parallel build..." -ForegroundColor Yellow
    try {
        cmake --build . --config Release
        if ($LASTEXITCODE -ne 0) {
            throw "Build failed with exit code $LASTEXITCODE"
        }
    } catch {
        Write-Host "Build failed completely. Check the error messages above." -ForegroundColor Red
        Set-Location ..
        exit 1
    }
}

Write-Host ""
Write-Host "Build completed successfully!" -ForegroundColor Green
Write-Host "Executable location varies by generator:" -ForegroundColor Blue
Write-Host "  Visual Studio: build\Release\ddogreen.exe" -ForegroundColor Yellow
Write-Host "  MinGW:        build\ddogreen.exe" -ForegroundColor Yellow
Write-Host ""
Write-Host "To test the binary:" -ForegroundColor Blue
Write-Host "  .\Release\ddogreen.exe --help    # For Visual Studio builds" -ForegroundColor Yellow
Write-Host "  .\ddogreen.exe --help            # For MinGW builds" -ForegroundColor Yellow
Write-Host ""
Write-Host "Note: Administrative privileges may be required for system monitoring features." -ForegroundColor Cyan

# Return to original directory
Set-Location ..
