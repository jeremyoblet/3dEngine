#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIRECTORY="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

"${SCRIPT_DIRECTORY}/install_linux_dependencies.sh"
"${SCRIPT_DIRECTORY}/install_vcpkg.sh"

echo "Environment setup completed."