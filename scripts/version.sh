#!/bin/bash

OUTPUT=$1

VERSION_MAJOR="0"
VERSION_MINOR="0"
GIT_REVISION="$(git rev-parse --short HEAD)"
DATE="$(date)"

cat << EOF > "$OUTPUT"
/* This file was generated automatically */

#pragma once
const int DUCKOS_VERSION_MAJOR = ${VERSION_MAJOR};
const int DUCKOS_VERSION_MINOR = ${VERSION_MINOR};
const char* DUCKOS_VERSION_STRING = "${VERSION_MAJOR}.${VERSION_MINOR}";
const char* DUCKOS_REVISION = "${GIT_REVISION}";
EOF
