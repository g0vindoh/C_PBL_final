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

# ── Build rules ──────────────────────────────────────────────────────
.PHONY: all clean run dirs

all: dirs $(EXE)

dirs:
	$(MKDIR)

$(EXE): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(EXE) $(LDFLAGS)
	@echo "Build successful: $(EXE)"

run: all
	$(RUN)

clean:
	$(RM) $(EXE)
	@echo "Cleaned."
