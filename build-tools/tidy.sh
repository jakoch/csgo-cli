#!/usr/bin/env bash

# -----------------------------------------------------------------------------
# clang-tidy helper
#
# Purpose
# - Run clang-tidy with the project's compile database and repo file layout.
# - Support full-repo checks or a focused single-file check.
#
# Requirements
# - A configured build directory containing compile_commands.json.
# - clang-tidy version 17 or newer available on PATH (or via CLANG_TIDY env).
#
# Usage
# - ./build-tools/tidy.sh
#     Runs clang-tidy on tracked C++ files under src/ and tests/.
# - ./build-tools/tidy.sh path/to/file.cpp
#     Runs clang-tidy only on the provided file.
#
# The build directory defaults to ./out/build/clang22-x64-linux-dbg-cov.
# Override with the BUILD_DIR environment variable, e.g.:
#    BUILD_DIR=/path/to/build ./build-tools/tidy.sh
#
# Notes
# - The highest installed clang-tidy version (>=17) is used, preferring
#   versioned binaries (clang-tidy-22, ...) over the plain `clang-tidy`.
#   Override with the CLANG_TIDY environment variable.
# - The clang-tidy exit code is propagated.
# -----------------------------------------------------------------------------

set -euo pipefail

if [[ $# -gt 1 ]]; then
  echo "Usage: $0 [FILE]"
  exit 1
fi

PRESET="clang22-x64-linux-dbg-cov"
MIN_CLANG_TIDY_VERSION="17"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
# Allow overriding build directory via env var BUILD_DIR for CI or local presets.
BUILD_DIR="${BUILD_DIR:-${REPO_ROOT}/out/build/${PRESET}}"
COMPILE_COMMANDS="${BUILD_DIR}/compile_commands.json"

# Ensure the compile database exists.
if [[ ! -f "$COMPILE_COMMANDS" ]]; then
  echo "Please run CMake configure first to generate compile_commands.json in ${BUILD_DIR}."
  exit 1
fi

get_version() {
  local binary="$1"
  command -v "$binary" >/dev/null 2>&1 || return 1
  local version
  # clang-tidy reports either "Ubuntu clang-tidy version X.Y" or "LLVM version X.Y".
  version=$("$binary" --version 2>/dev/null | sed -nE 's/.*(clang-tidy|LLVM) version ([0-9]+(\.[0-9]+)*).*/\2/p' | head -n 1)
  if [[ -n "$version" ]]; then
    echo "$version"
    return 0
  fi
  return 1
}

version_ge() {
  # Returns 0 (true) if $1 >= $2, using version sort.
  local min
  min=$(printf '%s\n' "$1" "$2" | sort -V | head -n1)
  [[ "$min" == "$2" ]]
}

check_version() {
  local binary="$1"
  local version
  version=$(get_version "$binary") || return 1
  if version_ge "$version" "$MIN_CLANG_TIDY_VERSION"; then
    echo "$binary"
    return 0
  fi
  return 1
}

find_installed() {
  # List every clang-tidy binary on PATH, deduplicated by name.
  # Versioned names (clang-tidy-*) are listed first, plain `clang-tidy` last,
  # so that ties on version resolve to the more specific binary.
  local dir bin name
  local -a found=()
  while IFS= read -r dir; do
    [[ -d "$dir" ]] || continue
    for bin in "$dir"/clang-tidy*; do
      [[ -x "$bin" ]] || continue
      name=${bin##*/}
      case " ${found[*]} " in
        *" $name "*) ;;
        *) found+=("$name") ;;
      esac
    done
  done < <(tr ':' '\n' <<<"$PATH")
  for name in "${found[@]}"; do
    [[ "$name" != "clang-tidy" ]] && echo "$name"
  done
  for name in "${found[@]}"; do
    [[ "$name" == "clang-tidy" ]] && echo "$name"
  done
}

find_latest() {
  # Pick the highest installed clang-tidy version (>= MIN_CLANG_TIDY_VERSION).
  local best="" best_version="" cand version
  while IFS= read -r cand; do
    version=$(get_version "$cand") || continue
    version_ge "$version" "$MIN_CLANG_TIDY_VERSION" || continue
    if [[ -z "$best" ]]; then
      best="$cand"
      best_version="$version"
    elif version_ge "$version" "$best_version" && [[ "$version" != "$best_version" ]]; then
      best="$cand"
      best_version="$version"
    fi
  done < <(find_installed)
  if [[ -n "$best" ]]; then
    echo "$best"
    return 0
  fi
  return 1
}

if [[ -n "${CLANG_TIDY:-}" ]]; then
  if ! CLANG_TIDY=$(check_version "$CLANG_TIDY"); then
    echo "Error: Requested clang-tidy '${CLANG_TIDY}' is unavailable or unsupported (need >=${MIN_CLANG_TIDY_VERSION})."
    exit 1
  fi
else
  if ! CLANG_TIDY=$(find_latest); then
    echo "Error: No compatible clang-tidy version (>=${MIN_CLANG_TIDY_VERSION}) found."
    exit 1
  fi
fi

echo "Using $("$CLANG_TIDY" --version | head -n 1)"

cd "$REPO_ROOT"

# Single file mode or default repo scan.
if [[ $# -eq 1 ]]; then
  if [[ ! -f "$1" ]]; then
    echo "File not found: $1"
    exit 1
  fi
  files=("$1")
else
  # Tracked sources under src/ and tests/.
  mapfile -t tracked < <(git ls-files 'src/*.cpp' 'src/*.h' 'src/*.hpp' 'tests/*.cpp' 'tests/*.h' 'tests/*.hpp')

  # Only analyze files that are part of the configured build (i.e. present in
  # compile_commands.json). This skips platform-specific sources that cannot be
  # compiled on the current host (e.g. src/platform/windows/* on Linux) and
  # standalone headers, which are covered transitively via their .cpp TUs.
  files=()
  for f in "${tracked[@]}"; do
    if grep -Fq -- "\"$REPO_ROOT/$f\"" "$COMPILE_COMMANDS" || grep -Fq -- "\"$f\"" "$COMPILE_COMMANDS"; then
      files+=("$f")
    fi
  done

  if [[ ${#files[@]} -eq 0 ]]; then
    echo "No C++ source files found in the compile database."
    exit 0
  fi
fi

# Run clang-tidy against the compile database from the configured build dir.
# compile_commands.json already contains every include path used at build time
# (vcpkg_installed, dependencies/sw_sdk, generated protobuf headers, ...).
"$CLANG_TIDY" -p "$BUILD_DIR" "${files[@]}"
