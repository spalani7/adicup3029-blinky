
cmake_minimum_required(VERSION 3.0.2)

set(PROJ_VERSION_MAJOR 0)
set(PROJ_VERSION_MINOR 0)
set(PROJ_VERSION_PATCH 4)
set(PROJ_VERSION_TWEAK 0)
set(PROJ_VERSION ${PROJ_VERSION_MAJOR}.${PROJ_VERSION_MINOR}.${PROJ_VERSION_PATCH}.${PROJ_VERSION_TWEAK} )

project(aducm3029ex	VERSION ${PROJ_VERSION} LANGUAGES C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,--gc-sections ")

set(APP_PATH "${BSP}/Boards/EVAL-ADICUP3029/Examples/drivers/gpio/blinky_example")
set(SYS_STARTUP_PATH "${DFP}/Source")
set(LINKER_SCRIPT "${SYS_STARTUP_PATH}/GCC/ADuCM3029.ld")
set(CMAKE_EXE_LINKER_FLAGS "-T ${LINKER_SCRIPT}")

# collect sources and add target
set(SRCS "${SYS_STARTUP_PATH}/adi_cyclecount.c" "${SYS_STARTUP_PATH}/GCC/startup_ADuCM3029.c" "${SYS_STARTUP_PATH}/system_ADuCM3029.c" "${SYS_STARTUP_PATH}/common.c")
set(SRCS ${SRCS} "${SYS_STARTUP_PATH}/drivers/pwr/adi_pwr.c" "${SYS_STARTUP_PATH}/drivers/gpio/adi_gpio.c")
add_executable(blinky.elf "${APP_PATH}/blinky_example.c" ${SRCS})

# generate bin/hex output
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Map=${PROJECT_BINARY_DIR}/blinky.map")
set(HEX_FILE ${PROJECT_BINARY_DIR}/blinky.hex)
set(BIN_FILE ${PROJECT_BINARY_DIR}/blinky.bin)
add_custom_command(TARGET blinky.elf POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -Oihex $<TARGET_FILE:blinky.elf> ${HEX_FILE}
        COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:blinky.elf> ${BIN_FILE}
        COMMENT "Building ${HEX_FILE} \nBuilding ${BIN_FILE}")