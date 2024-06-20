#!/usr/bin/env bash

# allow ENV.CLANG_FORMAT to define the path to the binary or default to clang-format
CLANG_FORMAT=${CLANG_FORMAT:-clang-format}

# require clang-format 17 to have consistent formatting features
$CLANG_FORMAT --version
if [[ ! $($CLANG_FORMAT --version) =~ "version 17" ]]; then
    echo "Error: clang-format version must be 17"
    exit 1
fi

find src -name *.h -o -name *.cpp -exec dos2unix {} \;
find src -name *.h -o -name *.cpp|xargs $CLANG_FORMAT -i -style=file
