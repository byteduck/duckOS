#!/bin/sh
set -e
runpath=$(cd -P -- "$(dirname -- "$0")" && pwd -P)
sudo -E SOURCE_DIR="$SOURCE_DIR" "$runpath/makeImage.sh"