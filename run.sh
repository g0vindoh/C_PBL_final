#!/usr/bin/env bash
# ──────────────────────────────────────────────────────
#  run.sh  –  One-click build & run (Linux / macOS)
#  Usage:  ./run.sh
# ──────────────────────────────────────────────────────
set -e

check_sdl2() {
    pkg-config --exists sdl2 2>/dev/null || \
    sdl2-config --version >/dev/null 2>&1
}

install_sdl2() {
    echo "SDL2 not found. Attempting auto-install..."
    if command -v apt-get >/dev/null 2>&1; then
        sudo apt-get install -y libsdl2-dev
    elif command -v brew >/dev/null 2>&1; then
        brew install sdl2
    elif command -v pacman >/dev/null 2>&1; then
        sudo pacman -S --noconfirm sdl2
    elif command -v dnf >/dev/null 2>&1; then
        sudo dnf install -y SDL2-devel
    else
        echo ""
        echo "ERROR: Could not auto-install SDL2."
        echo "Please install it manually:"
        echo "  Ubuntu/Debian : sudo apt install libsdl2-dev"
        echo "  macOS         : brew install sdl2"
        echo "  Arch          : sudo pacman -S sdl2"
        echo "  Fedora        : sudo dnf install SDL2-devel"
        exit 1
    fi
}

if ! check_sdl2; then
    install_sdl2
fi

make run
