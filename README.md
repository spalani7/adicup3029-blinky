# adicup3029-blinky
A simple blinky example using CMake for EVAL-ADICUP3029 platform


## Download `arm-none-eabi` cross compiler

* [From Arm](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads)
* [From xPack](https://xpack.github.io/arm-none-eabi-gcc/install/)
* [From Launchpad](https://launchpad.net/gcc-arm-embedded)


## CMSIS Pack, Device Family Pack (DFP) and Board Support Pack (BSP)

* [All CMSIS releases](https://github.com/ARM-software/CMSIS_5/releases/)

* [Download CMSIS 5.4.0](https://github.com/ARM-software/CMSIS_5/releases/download/5.4.0/ARM.CMSIS.5.4.0.pack)

* [All DFP and BSP releases](https://developer.arm.com/tools-and-software/embedded/cmsis/cmsis-packs)

* [Download DFP 3.2.0](http://download.analog.com/tools/EZBoards/CM302x/Releases/AnalogDevices.ADuCM302x_DFP.3.2.0.pack)

* [Download BSP 1.1.0](http://download.analog.com/tools/EZBoards/ADICUP3029/Releases/AnalogDevices.EVAL-ADICUP3029_BSP.1.1.0.pack)

`Note:` The above packs are downloaded as `*.pack`. This can be extracted using any archive file extractor similar to zip.


## Build
```bash
$ mkdir build && cd build
$ cmake -DCMAKE_TOOLCHAIN_FILE=../aducm3029.cmake ..
$ make
```

## License

MIT License