@echo off
REM Script de compilación para NERA ESP32-S3 Smartwatch
REM Inicializa el ambiente de ESP-IDF y compila el proyecto

setlocal enabledelayedexpansion

echo.
echo ============================================
echo   NERA Smartwatch Build Script
echo ============================================
echo.

REM Set ESP-IDF environment - matching Espressif installer
set IDF_PATH=C:\esp\v6.0.2\esp-idf
set IDF_TOOLS_PATH=C:\Espressif\tools
set IDF_PYTHON_ENV_PATH=%IDF_TOOLS_PATH%\python\v6.0.2\venv
set PYTHON=%IDF_PYTHON_ENV_PATH%\Scripts\python.exe
set ESP_ROM_ELF_DIR=%IDF_TOOLS_PATH%\esp-rom-elfs\20241011
set ESP_IDF_VERSION=6.0.2
set PYTHONPATH=%IDF_PATH%\tools;%PYTHONPATH%
set PATH=%IDF_PYTHON_ENV_PATH%\Scripts;%IDF_TOOLS_PATH%\cmake\4.0.3\bin;%IDF_TOOLS_PATH%\ninja\1.12.1;%IDF_TOOLS_PATH%\xtensa-esp-elf\esp-15.2.0_20251204\xtensa-esp-elf\bin;%IDF_TOOLS_PATH%\xtensa-esp-elf-gdb\esp-15.2.0_20251204\xtensa-esp-elf-gdb\bin;%IDF_TOOLS_PATH%\riscv32-esp-elf\esp-15.2.0_20251204\riscv32-esp-elf\bin;%IDF_TOOLS_PATH%\riscv32-esp-elf-gdb\esp-15.2.0_20251204\riscv32-esp-elf-gdb\bin;%PATH%

cd /d %~dp0

echo [INFO] IDF_PATH: %IDF_PATH%
echo [INFO] Working directory: %CD%
echo.

echo [1/3] Cleaning build artifacts...
if /I "%~1"=="clean" (
    if exist build rmdir /s /q build
    echo [INFO] Build directory cleaned.
) else (
    echo [INFO] Reusing existing build directory.
)

echo [2/3] Building project with ESP-IDF...
"%PYTHON%" "%IDF_PATH%\tools\idf.py" build

echo.
if %ERRORLEVEL% EQU 0 (
    echo.
    echo ============================================
    echo   BUILD SUCCESSFUL!
    echo ============================================
    echo Build artifacts at: %CD%\build
    echo Firmware: %CD%\build\NERA.elf
    echo.
) else (
    echo.
    echo ============================================
    echo   BUILD FAILED!
    echo ============================================
    echo.
    exit /b 1
)

endlocal
