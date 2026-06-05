# ─────────────────────────────────────────────────────────────────────
# Makefile – SmartTrafficSystem
# Supports: Windows (MinGW/MSYS2) and Linux/macOS
# ─────────────────────────────────────────────────────────────────────

CC      = gcc
TARGET  = SmartTrafficSystem

# Source files
SRC = src/main.c       \
      src/utils.c      \
      src/vehicle.c    \
      src/signal.c     \
      src/accident.c   \
      src/traffic.c    \
      src/ui.c         \
      src/simulation.c

# Include directories
INC = -Iinclude

# ── Platform detection ───────────────────────────────────────────────
ifeq ($(OS),Windows_NT)
    # Windows – MinGW / MSYS2 (SDL2 installed via pacman or manual)
    SDL_INC  = -IC:/msys64/mingw64/include/SDL2
    SDL_LIB  = -LC:/msys64/mingw64/lib -lSDL2main -lSDL2
    LDFLAGS  = -mwindows $(SDL_LIB) -lm
    CFLAGS   = -Wall -Wextra -O2 $(INC) $(SDL_INC) -DSDL_MAIN_HANDLED
    EXE      = $(TARGET).exe
    MKDIR    = if not exist logs mkdir logs
    RM       = del /Q
    RUN      = $(EXE)
else
    # Linux / macOS – SDL2 via apt / brew
    SDL_CFLAGS := $(shell sdl2-config --cflags)
    SDL_LIBS   := $(shell sdl2-config --libs)
    CFLAGS   = -Wall -Wextra -O2 $(INC) $(SDL_CFLAGS)
    LDFLAGS  = $(SDL_LIBS) -lm
    EXE      = $(TARGET)
    MKDIR    = mkdir -p logs
    RM       = rm -f
    RUN      = ./$(EXE)
endif

# ── SDL2 static build paths (Windows only) ──────────────────────────
# Download SDL2-2.30.6 MinGW dev package from:
#   https://github.com/libsdl-org/SDL/releases/tag/release-2.30.6
# and extract to C:/SDL2-2.30.6 (or adjust SDL_STATIC_* below).
SDL_STATIC_INC = C:/Users/hp/Downloads/SDL2-2.30.6/x86_64-w64-mingw32/include/SDL2
SDL_STATIC_LIB = C:/Users/hp/Downloads/SDL2-2.30.6/x86_64-w64-mingw32/lib
ISCC            = $(LOCALAPPDATA)/Programs/Inno Setup 6/ISCC.exe

# ── Build rules ──────────────────────────────────────────────────────
.PHONY: all clean run dirs release installer

all: dirs $(EXE)

dirs:
	$(MKDIR)

$(EXE): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(EXE) $(LDFLAGS)
	@echo "Build successful: $(EXE)"

run: all
	$(RUN)

# Portable single-EXE release: SDL2 statically linked, no DLL needed.
# Run:  make release   (Windows only)
release: dirs
	C:/mingw64/bin/gcc.exe -Wall -Wextra -O2 \
	    -Iinclude -I$(SDL_STATIC_INC) -DSDL_MAIN_HANDLED \
	    $(SRC) -o SmartTrafficSystem_portable.exe \
	    -L$(SDL_STATIC_LIB) -lmingw32 -mwindows \
	    -Wl,-Bstatic -lSDL2main -lSDL2 -Wl,-Bdynamic \
	    -ldinput8 -ldxguid -ldxerr8 \
	    -luser32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 \
	    -lshell32 -lsetupapi -lversion -luuid \
	    -static-libgcc -lm
	@echo "Portable EXE: SmartTrafficSystem_portable.exe"

# Build the Windows installer (requires: make release, Inno Setup 6)
installer: release
	"$(ISCC)" installer/SmartTraffic.iss
	@echo "Installer: installer/Output/SmartTrafficSystem_Setup.exe"

clean:
	$(RM) $(EXE)
	@echo "Cleaned."
