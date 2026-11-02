@echo off
REM Build script for NERA that uses Espressif ESP-IDF tools
REM This configures the complete environment needed for ESP32-S3 compilation

echo.
echo ================================================
echo   NERA Smartwatch - Build Script
echo ================================================
echo.

REM Get the directory where this script is located
set SCRIPT_DIR=%~dp0

REM Set up paths for Espressif tools
set ESP_IDF_TOOLS_PATH=C:\Espressif\tools
set IDF_PATH=C:\esp\v6.0.2\esp-idf
set IDF_PYTHON_ENV_PATH=%ESP_IDF_TOOLS_PATH%\python\v6.0.2\venv
set PYTHON=%IDF_PYTHON_ENV_PATH%\Scripts\python.exe

REM Configure PATH with all necessary tools
REM This is critical - the order matters!
set PATH=%IDF_PYTHON_ENV_PATH%\Scripts
set PATH=%PATH%;%ESP_IDF_TOOLS_PATH%\cmake\4.0.3\bin
set PATH=%PATH%;%ESP_IDF_TOOLS_PATH%\ninja\1.12.1
set PATH=%PATH%;%ESP_IDF_TOOLS_PATH%\xtensa-esp-elf\esp-15.2.0_20251204\xtensa-esp-elf\bin
set PATH=%PATH%;%ESP_IDF_TOOLS_PATH%\riscv32-esp-elf\esp-15.2.0_20251204\riscv32-esp-elf\bin
set PATH=%PATH%;C:\Program Files\Git\cmd
set PATH=%PATH%;%SystemRoot%\System32
set PATH=%PATH%;%SystemRoot%

REM Environment variables for idf.py
set ESP_IDF_VERSION=6.0.2
set PYTHONPATH=%IDF_PATH%\tools;%PYTHONPATH%

REM Verify that python exists
if not exist "%PYTHON%" (
    echo ERROR: Python not found at %PYTHON%
    echo Please ensure ESP-IDF is properly installed
    exit /b 1
)

REM Verify that idf.py exists
if not exist "%IDF_PATH%\tools\idf.py" (
    echo ERROR: idf.py not found at %IDF_PATH%\tools\idf.py
    exit /b 1
)

echo [INFO] Python: %PYTHON%
echo [INFO] IDF_PATH: %IDF_PATH%
echo [INFO] Working directory: %SCRIPT_DIR%
echo.

REM Clean build if requested
if exist build (
    echo [INFO] Cleaning previous build artifacts...
    rmdir /s /q build >nul 2>&1
)

echo [INFO] Building NERA firmware...
echo.

REM Execute the build
cd /d "%SCRIPT_DIR%"
"%PYTHON%" "%IDF_PATH%\tools\idf.py" build

REM Check result
if %ERRORLEVEL% EQU 0 (
    echo.
    echo ================================================
    echo   BUILD SUCCESSFUL!
    echo ================================================
    echo Build output at: %SCRIPT_DIR%\build
    echo Firmware file: %SCRIPT_DIR%\build\NERA.elf
    echo Partition table: %SCRIPT_DIR%\build\NERA.bin
    echo.
    exit /b 0
) else (
    echo.
    echo ================================================
    echo   BUILD FAILED (exit code: %ERRORLEVEL%)
    echo ================================================
    echo.
    echo Check the logs for details:
    echo - %SCRIPT_DIR%\build\log\idf_py_stderr_output_*
    echo - %SCRIPT_DIR%\build\log\idf_py_stdout_output_*
    echo.
    exit /b %ERRORLEVEL%
)
