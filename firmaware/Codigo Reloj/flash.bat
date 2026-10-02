@echo off
REM Flasheo y monitor serial de NERA con el entorno ESP-IDF 6.0.2.

setlocal
set IDF_PATH=C:\esp\v6.0.2\esp-idf
set IDF_TOOLS_PATH=C:\Espressif\tools
set IDF_PYTHON_ENV_PATH=%IDF_TOOLS_PATH%\python\v6.0.2\venv
set ESP_IDF_VERSION=6.0.2
set ESP_ROM_ELF_DIR=%IDF_TOOLS_PATH%\esp-rom-elfs\20241011
set PYTHON=%IDF_PYTHON_ENV_PATH%\Scripts\python.exe
set PATH=%IDF_PYTHON_ENV_PATH%\Scripts;%IDF_TOOLS_PATH%\cmake\4.0.3\bin;%IDF_TOOLS_PATH%\ninja\1.12.1;%IDF_TOOLS_PATH%\xtensa-esp-elf\esp-15.2.0_20251204\xtensa-esp-elf\bin;%IDF_TOOLS_PATH%\riscv32-esp-elf\esp-15.2.0_20251204\riscv32-esp-elf\bin;%PATH%

cd /d "%~dp0"
"%PYTHON%" "%IDF_PATH%\tools\idf.py" flash monitor
set EXIT_CODE=%ERRORLEVEL%
endlocal & exit /b %EXIT_CODE%
