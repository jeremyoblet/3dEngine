#!/usr/bin/env bash

set -euo pipefail

readonly VCPKG_DIRECTORY="${VCPKG_ROOT:-${HOME}/dev/vcpkg}"

if [ -d "${VCPKG_DIRECTORY}" ]; then
    echo "vcpkg already installed at ${VCPKG_DIRECTORY}."
    exit 0
fi

mkdir -p "$(dirname "${VCPKG_DIRECTORY}")"

git clone https://github.com/microsoft/vcpkg.git "${VCPKG_DIRECTORY}"

"${VCPKG_DIRECTORY}/bootstrap-vcpkg.sh"