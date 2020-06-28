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

set(TOOLS_ROOT /Users/spalani/Downloads/gcc-arm-none-eabi-9-2019-q4-major)
set(TRIPLE arm-none-eabi)

set(CMAKE_C_COMPILER ${TOOLS_ROOT}/bin/${TRIPLE}-gcc)
set(CMAKE_CXX_COMPILER ${TOOLS_ROOT}/bin/${TRIPLE}-g++)
# set(CMAKE_SYSROOT ${tools}) # optional, and may be specified if a sysroot is available.

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(PACK_ROOT "/Users/spalani/embroot/")
set(CSP "${PACK_ROOT}/ARM.CMSIS.5.4.0")
set(BSP "${PACK_ROOT}/AnalogDevices.EVAL-ADICUP3029_BSP.1.1.0")
set(DFP "${PACK_ROOT}/AnalogDevices.ADuCM302x_DFP.3.2.0")

include_directories(${DFP}/Include)
include_directories(${DFP}/Include/config)
include_directories(${CSP}/CMSIS/Core/Include/)

set(CMAKE_C_FLAGS "-Wall -ffunction-sections -fdata-sections -ffreestanding -fno-common") #  -DCORE0 -D_RTE_
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=${MCU_ARCH} -mthumb -mfloat-abi=${MCU_FLOAT_ABI} -D__ADUCM3029__ ") #  -DCORE0 -D_RTE_

if (MCU_FLOAT_ABI STREQUAL hard)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mfpu=${MCU_FPU}")
endif ()

set(CMAKE_EXE_LINKER_FLAGS "--specs=nosys.specs" CACHE INTERNAL "")