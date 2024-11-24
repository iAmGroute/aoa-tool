#!/dev/null # source me

[ ! "$_IMPORT_REF_PATH" ] || return 0
_IMPORT_REF_PATH="$PWD" # TODO: make associative array, per file

# Enable strict error handling
set   -o pipefail
shopt -s lastpipe
shopt -s inherit_errexit
export SHELLOPTS
trap 'echo "ERROR: $_IMPORT_REF_PATH/$BASH_SOURCE:$LINENO"' ERR

get_line() {
  local FNAME="$1"
  local NUM="$2"
  local LINE=''
  (
    while (( NUM-- )); do
      read -r LINE
    done
    echo "$LINE"
  ) <"$FNAME"
}

realdirpath() {
  # ideally, we would just use dirname of realpath -sm,
  # but busybox doesn't support it
  local T="$(dirname -- "$1")"
  if [[ "$T" != /* ]]; then
    T="$_IMPORT_REF_PATH/$T" # rebase
  fi
  echo "$(cd -- "$T"; pwd)"
}

realpath() {
  local T="$(realdirpath "$1")"
  echo "${T}/$(basename -- "$1")"
}

_trap_ERR() {
  echo "_trap_ERR" "$@" "${BASH_SOURCE[@]}" >&2
  local FILE="$(realpath "${BASH_SOURCE[1]}")"
  local LINE=$(get_line "$FILE" "$1" || true)
  echo "Error near ${FILE}:${1}: ${LINE}" >&2
  exit 1
}
trap '_trap_ERR $LINENO' ERR

declare -A _IMPORT_MAP

import() {
  local P="$(realdirpath "${BASH_SOURCE[1]}")/$1"
  if [ ! "${_IMPORT_MAP["$P"]}" ]; then
            _IMPORT_MAP["$P"]=y
    . "$P"
  fi
}
