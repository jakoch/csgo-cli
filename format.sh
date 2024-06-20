#!/usr/bin/env bash

# Allow ENV.CLANG_FORMAT to define the path to the binary or default to clang-format
CLANG_FORMAT=${CLANG_FORMAT:-clang-format}

# Check clang-format version
VERSION=$("$CLANG_FORMAT" --version)

# Require clang-format 17 or 18 for consistent formatting features
if [[ ! $VERSION =~ "version 17" && ! $VERSION =~ "version 18" ]]; then
    echo "Error: Unsupported clang-format version. Must be version 17 or 18."
    echo "Found version: $VERSION"
    exit 1
fi

find src -name *.h -o -name *.cpp -exec dos2unix {} \;
find src -name *.h -o -name *.cpp|xargs $CLANG_FORMAT -i -style=file
