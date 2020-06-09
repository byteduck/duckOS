CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0)
PROJECT(duckos)

ADD_SUBDIRECTORY(src/kernel/)

ADD_CUSTOM_TARGET(image
        COMMAND ${CMAKE_COMMAND} -E env "SOURCE_DIR=${CMAKE_SOURCE_DIR}" ${CMAKE_SOURCE_DIR}/scripts/image-grub.sh
        BYPRODUCTS ${CMAKE_BINARY_DIR}/duckOS.img
        USES_TERMINAL
)

ADD_CUSTOM_TARGET(qemu-kernel
        COMMAND ${CMAKE_SOURCE_DIR}/scripts/qemu-kernel.sh
        USES_TERMINAL
)

ADD_CUSTOM_TARGET(qemu-image
        COMMAND ${CMAKE_SOURCE_DIR}/scripts/qemu-image.sh
        USES_TERMINAL
)