#!/dev/null # source me

cd_mydir() {
  cd -- "$(dirname -- "${BASH_SOURCE[1]}")"
}
