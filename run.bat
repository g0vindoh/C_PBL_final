@echo off
REM ──────────────────────────────────────────────────────
REM  run.bat  –  One-click build & run (Windows / MSYS2)
REM  Double-click this file, or run from terminal.
REM ──────────────────────────────────────────────────────

REM Check if make is available (MSYS2/MinGW must be in PATH)
where make >nul 2>&1
if errorlevel 1 (
    echo.
    echo  ERROR: 'make' not found.
    echo.
    echo  You need MSYS2 installed with the MinGW64 toolchain.
    echo  Steps:
    echo    1. Download MSYS2 from https://www.msys2.org/
    echo    2. Open "MSYS2 MinGW 64-bit" terminal
    echo    3. Run: pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-SDL2 make
    echo    4. Add C:\msys64\mingw64\bin to your system PATH
    echo    5. Re-run this script
    echo.
    pause
    exit /b 1
)

where gcc >nul 2>&1
if errorlevel 1 (
    echo.
    echo  ERROR: 'gcc' not found. Make sure MSYS2 MinGW64 bin is in your PATH.
    echo  Default path: C:\msys64\mingw64\bin
    echo.
    pause
    exit /b 1
)

echo Building SmartTrafficSystem...
make run
if errorlevel 1 (
    echo.
    echo  Build failed. Check the output above for errors.
    pause
    exit /b 1
)
