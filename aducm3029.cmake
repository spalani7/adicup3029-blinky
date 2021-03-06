########################################################
## Cmake toolchain file for building MCSB project for ADuCM3029
##
## Copyright 2020 Analog Devices Inc.
## Author: Sundar Palani <sundar.palani@analog.com>
##
## Analog Devices Proprietary and Confidential
########################################################

set(MCU_ARCH cortex-m3)
set(MCU_FLOAT_ABI soft)
set(MCU_FPU fpv4-sp-d16)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_SYSTEM_PROCESSOR arm)

# automatically set to true if build platform is different from target platform
# SET(CMAKE_CROSSCOMPILING 1)

set(CC_ROOT $ENV{TOOLCHAINROOT})
if (NOT IS_DIRECTORY ${CC_ROOT})
    set(CC_ROOT /usr)
endif()
set(TRIPLE arm-none-eabi)

set(TRIPLE arm-none-eabi)

set(CMAKE_C_COMPILER ${CC_ROOT}/bin/${TRIPLE}-gcc)
set(CMAKE_CXX_COMPILER ${CC_ROOT}/bin/${TRIPLE}-g++)
# set(CMAKE_SYSROOT ${tools}) # optional, and may be specified if a sysroot is available.

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(CMAKE_C_FLAGS "-Wall -ffunction-sections -fdata-sections -ffreestanding -fno-common") #  -DCORE0 -D_RTE_
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=${MCU_ARCH} -mthumb -mfloat-abi=${MCU_FLOAT_ABI} -D__ADUCM3029__ ") #  -DCORE0 -D_RTE_

if (MCU_FLOAT_ABI STREQUAL hard)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mfpu=${MCU_FPU}")
endif ()

set(CMAKE_EXE_LINKER_FLAGS "--specs=rdimon.specs" CACHE INTERNAL "")
