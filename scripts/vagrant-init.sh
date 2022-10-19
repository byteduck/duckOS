cd /vagrant

sudo locale-gen UTF-8

sudo apt-get install software-properties-common -y
sudo add-apt-repository ppa:george-edison55/cmake-3.x -y
sudo apt update -y
sudo apt install build-essential cmake bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo qemu-system-i386 qemu-utils nasm gawk grub2-common grub-pc rsync -y

export CMAKE_ASM_NASM_COMPILER=nasm

cd toolchain
chmod a+x ./build-toolchain.sh
bash build-toolchain.sh

cd ../
mkdir -p cmake-build
cd cmake-build
cmake .. -DCMAKE_TOOLCHAIN_FILE=toolchain/CMakeToolchain.txt