#!/dev/null # source me

which_docker() {
  local R=''
  if   command -v podman >/dev/null; then R=podman
  elif command -v docker >/dev/null; then R=docker
  else
    echo 'Error: podman or docker is required, but none found' >&2
    return 1
  fi
  echo "Using: $R" >&2
  echo "$R"
  return 0
}
