#!/bin/bash
. "$(dirname -- "${BASH_SOURCE[0]}")/../import.sh" || exit 1

import ../common.sh
import ../docker.sh

cd_mydir
cd ..

if [ "$1" != '--container' ]; then
  DOCKER=$(which_docker)
  set -x
  exec "$DOCKER" run -i --rm --init -v .:/wd --workdir /wd aoa-tool-buildenv:latest /wd/build/build.sh --container
fi

cd scrcpy-for-aoa/
rm -rf builddir
meson setup builddir
cd ../aoa-tool/
make clean
make
