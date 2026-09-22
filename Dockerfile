# ╔══════════════════════════════════════════════════════════════════════════╗
# ║               ARISE — Arch Linux Build & Test Container                  ║
# ╚══════════════════════════════════════════════════════════════════════════╝
#
# Mirrors the exact target OS. Builds the project and runs all GTest suites.
# Usage:
#   docker build -t arise-test .
#   docker run --rm arise-test            # runs ctest automatically
#   docker run --rm -it arise-test bash   # drop into a shell for exploration

FROM archlinux:latest

LABEL maintainer="ARISE Project"
LABEL description="Arch Linux build and test environment for ARISE"

# ── 1. Update package database and install build tools ───────────────────────
RUN pacman -Syu --noconfirm && \
    pacman -S --noconfirm \
        base-devel \
        cmake \
        git \
        curl \
        python \
        nlohmann-json \
        alsa-lib \
        portaudio \
    && pacman -Scc --noconfirm

# ── 2. Copy project source ────────────────────────────────────────────────────
WORKDIR /arise
COPY . .

# ── 3. Configure and build (CMake fetches whisper.cpp & GTest automatically) ──
RUN cmake -S . -B build \
        -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build --parallel "$(nproc)"

# ── 4. Default command: run all unit tests via CTest ──────────────────────────
WORKDIR /arise/build
CMD ["ctest", "--output-on-failure", "--test-dir", "."]
