# Shared settings for the scripts in this folder.
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"
case "$(uname -s)" in
  MINGW*|MSYS*|CYGWIN*) GENERATOR=(-G "MinGW Makefiles"); HOST_CC=(-DCMAKE_C_COMPILER=gcc) ;;
  *) GENERATOR=(); HOST_CC=() ;;
esac
