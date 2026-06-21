#!/usr/bin/env bash

set -euo pipefail

readonly VCPKG_DIRECTORY="${HOME}/dev/vcpkg"

if [ -d "${VCPKG_DIRECTORY}" ]; then
    echo "vcpkg already installed."
    exit 0
fi

mkdir -p "${HOME}/dev"

cd "${HOME}/dev"

git clone https://github.com/microsoft/vcpkg.git

cd vcpkg

./bootstrap-vcpkg.sh