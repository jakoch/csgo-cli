#!/usr/bin/env bash

set -euo pipefail

CLANG_FORMAT=${CLANG_FORMAT:-}
MIN_CLANG_FORMAT_VERSION="17"

get_version() {
  local binary="$1"
  command -v "$binary" >/dev/null 2>&1 || return 1
  local version
  version=$("$binary" --version 2>/dev/null | sed -nE 's/.*clang-format version ([0-9]+(\.[0-9]+)*).*/\1/p')
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
  if version_ge "$version" "$MIN_CLANG_FORMAT_VERSION"; then
    echo "$binary"
    return 0
  fi
  return 1
}

find_installed() {
  # List every clang-format binary on PATH, deduplicated by name.
  # Versioned names (clang-format-*) are listed first, plain `clang-format` last,
  # so that ties on version resolve to the more specific binary.
  local dir bin name
  local -a found=()
  while IFS= read -r dir; do
    [[ -d "$dir" ]] || continue
    for bin in "$dir"/clang-format*; do
      [[ -x "$bin" ]] || continue
      name=${bin##*/}
      case " ${found[*]} " in
        *" $name "*) ;;
        *) found+=("$name") ;;
      esac
    done
  done < <(tr ':' '\n' <<<"$PATH")
  for name in "${found[@]}"; do
    [[ "$name" != "clang-format" ]] && echo "$name"
  done
  for name in "${found[@]}"; do
    [[ "$name" == "clang-format" ]] && echo "$name"
  done
}

find_latest() {
  # Pick the highest installed clang-format version (>= MIN_CLANG_FORMAT_VERSION).
  local best="" best_version="" cand version
  while IFS= read -r cand; do
    version=$(get_version "$cand") || continue
    version_ge "$version" "$MIN_CLANG_FORMAT_VERSION" || continue
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

if [[ -n "$CLANG_FORMAT" ]]; then
  if ! CLANG_FORMAT=$(check_version "$CLANG_FORMAT"); then
    echo "Error: Requested clang-format '$CLANG_FORMAT' is unavailable or unsupported (need >=${MIN_CLANG_FORMAT_VERSION})."
    exit 1
  fi
else
  if ! CLANG_FORMAT=$(find_latest); then
    echo "Error: No compatible clang-format version (>=${MIN_CLANG_FORMAT_VERSION}) found."
    exit 1
  fi
fi

# Display the binary and version being used
VERSION=$("$CLANG_FORMAT" --version)
echo "Using clang-format ($VERSION)"

if [[ -z "${CI:-}" && -z "${GITHUB_ACTIONS:-}" ]]; then
  if ! command -v dos2unix >/dev/null 2>&1; then
    if command -v apt-get >/dev/null 2>&1; then
      echo "dos2unix not found, installing via apt-get..."
      sudo apt-get update
      sudo apt-get install -y dos2unix
    else
      echo "Error: dos2unix not found and apt-get is unavailable."
      exit 1
    fi
  fi
  find src tests -type f \( -name "*.h" -o -name "*.hpp" -o -name "*.cpp" \) -exec dos2unix {} \;
fi

find src tests -type f \( -name "*.h" -o -name "*.hpp" -o -name "*.cpp" \) -exec "$CLANG_FORMAT" -i -style=file {} \;

# In the CI context, we run `git diff --exit-code`.
# After clang-format finishes, we check for changes with `git diff`.
# If there are changes, we exit with a non-zero status code, causing the CI job to fail.
# This ensures that code formatting is enforced.
#  --ignore-file-mode is used to ignore chmod changes.
if [[ -n "${CI:-}" || -n "${GITHUB_ACTIONS:-}" ]]; then
  if ! git -c core.fileMode=false diff --exit-code; then
    echo "Error: Code formatting issues detected. Please run ./build-tools/format.sh and commit the changes."
    exit 1
  fi
fi
