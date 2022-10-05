#!/bin/bash
set -e
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
source "$DIR/../scripts/duckos.sh"
export PATH="/usr/local/opt/m4/bin:$PATH"
if [[ "$(uname -s)" != "Darwin" ]]; then
	fail "FUSE-ext2 is only needed on macOS."
fi
pushd "$DIR"/../cmake-build
if [ ! -d fuse-ext2 ]; then
	msg "Cloning alperakcanfuse-ext2..."
	git clone https://github.com/alperakcan/fuse-ext2.git
fi
cd fuse-ext2
msg "Preparing build..."
./autogen.sh
CFLAGS="-I/usr/local/include/osxfuse/ -I/$(brew --prefix e2fsprogs)/include" LDFLAGS="-L$(brew --prefix e2fsprogs)/lib" ./configure
msg "Building..."
make
msg "Installing..."
sudo make install
success "Installed FUSE-ext2!"
popd
