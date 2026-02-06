@echo off
REM ============================================================
REM  Godot 4.x GDExtension - Visual Studio Build Script (CMake)
REM  Author: Empty
REM  Version: 1.0
REM  Date: 2025-11-05
REM Features:
REM   1. Automatically build godot-cpp (required for GDExtension)
REM   2. Generate main project .sln / .vcxproj (first-time setup)
REM   3. Preserve existing Visual Studio configuration on re-run
REM   4. Create empty .gdignore files in 'out/' and 'godot-cppbuild/' to prevent Godot import errors
REM   5. Optional: 'clean' parameter forces full rebuild of build directories, cmd:GenerateProjectFiles.bat clean
REM ============================================================

setlocal enabledelayedexpansion

set PROJECT_NAME=HMSandbox
set ROOT_DIR=%~dp0
set BUILD_DIR=%ROOT_DIR%Build
set BUILD_TYPE=Debug
set GODOT_CPP_DIR=%ROOT_DIR%godot-cpp
set GODOT_CPP_BUILD=%ROOT_DIR%godot-cppbuild
set CMAKE_GENERATOR="Visual Studio 18 2026"
set CMAKE_ARCH=x64

echo.
echo ============================================================
echo Generating Visual Studio project for %PROJECT_NAME%
echo ============================================================

REM ------------------------------------------------------------
REM Check if CMake exists
REM ------------------------------------------------------------
where cmake >nul 2>nul
if errorlevel 1 (
    echo [ERROR] CMake not found. Please install CMake and add it to PATH.
    pause
    exit /b 1
)

REM ------------------------------------------------------------
REM Optional: clean old build directory
REM ------------------------------------------------------------
set CLEAN_BUILD=0
if /I "%1"=="clean" set CLEAN_BUILD=1

if %CLEAN_BUILD%==1 (
    if exist "%BUILD_DIR%" (
        echo [CLEAN] Removing old build directory...
        rmdir /s /q "%BUILD_DIR%"
    )
)


REM ------------------------------------------------------------
REM Build godot-cpp
REM ------------------------------------------------------------
cd /d "%GODOT_CPP_DIR%"
echo [INFO] Configuring godot-cpp...
cmake -B "%GODOT_CPP_DIR%\build" -G %CMAKE_GENERATOR% -A %CMAKE_ARCH% -DCMAKE_BUILD_TYPE=%BUILD_TYPE%
if errorlevel 1 (
    echo [ERROR] Failed to configure godot-cpp!
    pause
    exit /b 1
)

echo [INFO] Building godot-cpp...
cmake --build "%GODOT_CPP_DIR%\build" --config %BUILD_TYPE%
if errorlevel 1 (
    echo [ERROR] Failed to build godot-cpp!
    pause
    exit /b 1
)
echo [1/4] godot-cpp build completed.

REM ------------------------------------------------------------
REM Generate main project
REM ------------------------------------------------------------
set SOLUTION_FILE=%BUILD_DIR%\%PROJECT_NAME%.sln
if exist "%SOLUTION_FILE%" (
    echo [INFO] Existing solution detected: %SOLUTION_FILE%
    echo [SKIP] Skipping CMake configuration to preserve Visual Studio settings.
) else (
    cd /d "%ROOT_DIR%"
    echo [INFO] Configuring main project...
    cmake -B "%BUILD_DIR%" -G %CMAKE_GENERATOR% -A %CMAKE_ARCH% -DCMAKE_BUILD_TYPE=%BUILD_TYPE%
    if errorlevel 1 (
        echo [ERROR] Failed to configure main project!
        pause
        exit /b 1
    )
    echo [2/4] Main project CMake files generated.
)

REM ------------------------------------------------------------
REM Automatically create .gdignore files to prevent Godot importing build artifacts
REM ------------------------------------------------------------
echo [INFO] Creating .gdignore files for output directories...

REM Space-separated list of directories
set IGNORE_DIRS=%ROOT_DIR%Build %ROOT_DIR%bin %GODOT_CPP_DIR%\build

for %%D in (%IGNORE_DIRS%) do (
    if not exist "%%D" (
        echo [INFO] Creating directory: %%D
        mkdir "%%D" >nul 2>nul
    )
    set IGNORE_FILE=%%D\.gdignore
    if not exist "!IGNORE_FILE!" (
        echo.> "!IGNORE_FILE!"
        echo [INFO] Created .gdignore -> !IGNORE_FILE!
    ) else (
        echo [SKIP] .gdignore already exists: !IGNORE_FILE!
    )
)
echo [3/4] .gdignore files created.

REM ------------------------------------------------------------
REM Open Visual Studio solution
REM ------------------------------------------------------------
set SOLUTION_FILE=%BUILD_DIR%\%PROJECT_NAME%.sln
if exist "%SOLUTION_FILE%" (
    echo [INFO] Opening Visual Studio solution...
    start "" "%SOLUTION_FILE%"
) else (
    echo [WARNING] Solution file not found: %SOLUTION_FILE%
)

echo [4/4] Build script completed.
pause
