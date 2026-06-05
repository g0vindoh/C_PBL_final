@echo off
REM SmartTrafficSystem Release Builder
REM Produces a single EXE with SDL2 statically linked (no DLL needed).

SET GCC=C:\mingw64\bin\gcc.exe
SET SDL_INC=C:\Users\hp\Downloads\SDL2-2.30.6\x86_64-w64-mingw32\include\SDL2
SET SDL_LIB=C:\Users\hp\Downloads\SDL2-2.30.6\x86_64-w64-mingw32\lib
SET OUT=SmartTrafficSystem_portable.exe

SET SRCS=src/main.c src/utils.c src/vehicle.c src/signal.c src/accident.c src/traffic.c src/ui.c src/simulation.c

echo [1/2] Compiling and linking (static SDL2)...
"%GCC%" -Wall -Wextra -O2 ^
    -Iinclude -I"%SDL_INC%" -DSDL_MAIN_HANDLED ^
    %SRCS% ^
    -o "%OUT%" ^
    -L"%SDL_LIB%" ^
    -lmingw32 -mwindows ^
    -Wl,-Bstatic -lSDL2main -lSDL2 -Wl,-Bdynamic ^
    -ldinput8 -ldxguid -ldxerr8 ^
    -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 ^
    -lshell32 -lsetupapi -lversion -luuid ^
    -static-libgcc -lm

IF %ERRORLEVEL% NEQ 0 (
    echo [FAIL] Compile error. See above.
    exit /b 1
)

echo [2/2] Build complete.
FOR %%F IN ("%OUT%") DO echo     Output: %%~nxF  ^(%%~zF bytes^)
echo.
echo This EXE has SDL2 built-in. No SDL2.dll required.
echo Copy it anywhere and run directly.
