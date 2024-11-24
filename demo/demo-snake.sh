#!/bin/bash
. "$(dirname -- "${BASH_SOURCE[0]}")/../import.sh" || exit 1

import ../common.sh

cd_mydir
cd ..

echo 'This will emulate an AOA keyboard pressing the arrow keys in a random-looking order.'
echo 'Connect an Android device via USB, open the app drawer and scroll to the top, then press enter.'
read -r || true

exec ./run/run.sh "$@" < ./test_files/snake
