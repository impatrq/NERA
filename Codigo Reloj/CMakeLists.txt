# =============================================================================
# NERA Smartwatch — CMakeLists.txt raíz
# =============================================================================
#
# Este archivo es el punto de entrada del sistema de build CMake para ESP-IDF.
# Su función es incluir las herramientas de cmake de ESP-IDF y declarar el
# nombre del proyecto. La mayor parte de la configuración real de build
# ocurre en main/CMakeLists.txt.
#
# ESP-IDF usa CMake como sistema de build, lo que significa que antes de
# compilar, CMake genera los archivos de compilación reales (Makefiles o
# Ninja files) basándose en estos CMakeLists.txt.
# =============================================================================

cmake_minimum_required(VERSION 3.16)

# Incluye las utilidades de CMake de ESP-IDF.
# La variable $ENV{IDF_PATH} apunta al directorio de instalación de ESP-IDF.
# Si esta variable no está definida, la compilación fallará con un error claro.
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Nombre del proyecto. Este nombre aparecerá en el binario generado
# (NERA.bin) y en los logs de compilación.
project(NERA VERSION 0.1.0 LANGUAGES C CXX)
