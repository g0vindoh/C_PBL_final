# Smart Traffic Control System

A real-time traffic simulation with adaptive signals, emergency vehicles, accident detection, and a live dashboard. Built with C + SDL2.

---

## Quick Start

### Linux / macOS
```bash
./run.sh
```
That's it. The script auto-installs SDL2 if missing, builds, and launches.

### Windows
1. Install [MSYS2](https://www.msys2.org/)
2. Open **MSYS2 MinGW 64-bit** terminal and run:
   ```bash
   pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-SDL2 make
   ```
3. Add `C:\msys64\mingw64\bin` to your system `PATH`
4. Double-click `run.bat`

---

## Manual Build (any platform)

| Step | Command |
|------|---------|
| Build | `make` |
| Build & run | `make run` |
| Clean | `make clean` |

---

## Controls

| Key | Action |
|-----|--------|
| `Space` | Pause / Resume |
| `↑` / `↓` | Increase / Decrease traffic density |
| `A` | Trigger accident |
| `E` | Spawn emergency vehicle |
| `N` | Toggle night mode |
| `R` | Toggle rain |
| `F5` | Reset simulation |
| `Esc` | Quit |

---

## Dependencies

| Platform | Dependency | Install |
|----------|------------|---------|
| Linux (Debian/Ubuntu) | libsdl2-dev | `sudo apt install libsdl2-dev` |
| macOS | SDL2 | `brew install sdl2` |
| Arch Linux | sdl2 | `sudo pacman -S sdl2` |
| Windows | SDL2 (via MSYS2) | `pacman -S mingw-w64-x86_64-SDL2` |

---

## Logs

All events (spawns, accidents, pauses, resets) are written to `logs/traffic_log.txt`.
