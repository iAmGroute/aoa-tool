#!/bin/bash
. "$(dirname -- "${BASH_SOURCE[0]}")/../import.sh" || exit 1

import ../docker.sh

DOCKER=$(which_docker)
set -x
exec "$DOCKER" run -it --rm --init --device=/dev/bus/usb:/dev/bus/usb aoa-tool:latest aoa-tool "$@"
