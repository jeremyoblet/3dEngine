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
    libglu1-mesa-dev