#!/bin/bash
set -e
source "./toolchain-common.sh"

mkdir -p "$EDIT"
cd "$EDIT"
download-binutils use-git
download-gcc use-git
download-newlib use-git
cd "$DIR"

printf "Binutils and newlib downloaded and ready for editing in %s!\n" "$EDIT"
printf "DO NOT create any extra git commits in the created repositories or else gen-patches.sh will not work properly.\n"