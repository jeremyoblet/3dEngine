#!/usr/bin/env bash

set -euo pipefail

echo "Installing Linux system dependencies..."

sudo apt update

sudo apt install -y \
    build-essential \
    cmake \
    ninja-build \
    pkg-config \
    curl \
    zip \
    unzip \
    tar \
    git \
    xorg-dev \
    libxinerama-dev \
    libxcursor-dev \
    libxi-dev \
    libxrandr-dev \
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    libegl1-mesa-dev \
    libopengl-dev \
    libfontconfig1-dev \
    libfreetype6-dev \
    libdbus-1-dev \
    '^libxcb.*-dev' \
    libx11-xcb-dev \
    libxrender-dev \
    libxkbcommon-dev \
    libxkbcommon-x11-dev \
    libatspi2.0-dev \
    libglib2.0-dev \
    libharfbuzz-dev \
    libpcre2-dev \
    python3 \
    python3-venv \
    bison \
    flex \
    autoconf \
    autoconf-archive \
    automake \
    libtool