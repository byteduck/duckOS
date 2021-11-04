#!/bin/sh

err () {
  printf "\e[31m[ERROR] $1 \e[39m\n"
}

msg () {
  printf "\e[36m[INFO] $1 \e[39m\n"
}

warn () {
  printf "\e[33m[WARN] $1 \e[39m\n"
}

success () {
  printf "\e[32m[SUCCESS] $1 \e[39m\n"
}

fail () {
  err "$1"

  if [ -z "$FAILING" ]; then
    FAILING=1
    if [ ! -z "$ON_FAIL" ]; then
      $ON_FAIL
    fi
  fi

  if [ $? -eq 0 ]; then
    exit 1
  else
    exit $?
  fi
}