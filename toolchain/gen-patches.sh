#!/bin/bash
set -e
source "./toolchain-common.sh"

if [ ! "$1" ]; then
  printf "No repo specified. Please specify either binutils or gcc to generate a patch.\n"
  exit 1
fi

if [ "$1" == "binutils" ]; then
  cd "$EDIT/$BINUTILS_FILE" || exit 1
  git add -N .
  git diff > "$DIR/binutils-$BINUTILS_VERSION.patch" || exit 1
  cd "$BUILD"
  printf "binutils patch file generated at %s/binutils-%s.patch!\n" "$DIR" "$BINUTILS_VERSION"
elif [ "$1" == "gcc" ]; then
  cd "$EDIT/$GCC_FILE" || exit 1
  git add -N .
  git diff > "$DIR/gcc-$GCC_VERSION.patch" || exit 1
  cd "$BUILD"
  printf "gcc patch file generated at %s/gcc-%s.patch!\n" "$DIR" "$GCC_VERSION"
else
  printf "Unknown repo. Please specify either binutils or gcc.\n"
  exit 1
fi