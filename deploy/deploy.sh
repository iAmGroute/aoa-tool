#!/bin/bash
. "$(dirname -- "${BASH_SOURCE[0]}")/../import.sh" || exit 1

import ../common.sh
import ../docker.sh

cd_mydir

mv -v ../aoa-tool/main.out .

DOCKER=$(which_docker)
set -x
exec "$DOCKER" build -t aoa-tool:latest .
