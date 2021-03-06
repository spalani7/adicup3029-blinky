# ADICUP3029 blinky 

[![CI](https://github.com/spalani7/adicup3029-blinky/workflows/CI/badge.svg?branch=master)](https://github.com/spalani7/adicup3029-blinky/actions)

A simple blinky example using CMake for [EVAL-ADICUP3029](https://www.analog.com/en/design-center/evaluation-hardware-and-software/evaluation-boards-kits/EVAL-ADICUP3029.html) platform.

![](https://www.analog.com/-/media/analog/en/evaluation-board-images/images/eval-adicup3029-angle-web.gif?la=en&h=500&thn=1&hash=AC90B1ECD6EDB1A50AA636027C404D8F)

### Download `arm-none-eabi` cross compiler

* [From Arm](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads)
* [From xPack](https://xpack.github.io/arm-none-eabi-gcc/install/)
* [From Launchpad](https://launchpad.net/gcc-arm-embedded)
* Pre-compiled binaries from `apt` (only debian)
```bash
sudo apt-get install -y gcc-arm-none-eabi binutils-arm-none-eabi
```

### Download CMSIS Pack, Device Family Pack (DFP) and Board Support Pack (BSP)

* [Download CMSIS 5.4.0](https://github.com/ARM-software/CMSIS_5/releases/download/5.4.0/ARM.CMSIS.5.4.0.pack)
* [Download DFP 3.2.0](http://download.analog.com/tools/EZBoards/CM302x/Releases/AnalogDevices.ADuCM302x_DFP.3.2.0.pack)
* [Download BSP 1.1.0](http://download.analog.com/tools/EZBoards/ADICUP3029/Releases/AnalogDevices.EVAL-ADICUP3029_BSP.1.1.0.pack)

* [Other CMSIS releases](https://github.com/ARM-software/CMSIS_5/releases/)
* [Other DFP and BSP releases](https://developer.arm.com/tools-and-software/embedded/cmsis/cmsis-packs)

`Note:` The above packs (`*.pack`), can be extracted using any archive file extractor similar to zip.

### Extract packs

```bash
$ cd adicup3029-blinky
$ unzip ARM.CMSIS.5.4.0.pack -d ARM.CMSIS.5.4.0
$ unzip AnalogDevices.ADuCM302x_DFP.3.2.0.pack -d AnalogDevices.ADuCM302x_DFP.3.2.0
$ unzip AnalogDevices.EVAL-ADICUP3029_BSP.1.1.0.pack -d AnalogDevices.EVAL-ADICUP3029_BSP.1.1.0
```

### Set environment variables
For Debian linux or mac OSX:  
```
export TOOLCHAINROOT="<path to cross compiler directory containing 'bin' directory>"
export CMSISROOT="<path to directory containing CMSIS pack>"
export ADIPACKSROOT="<path to directory BSP and DFP packs>"
```

Default values:  
```
TOOLCHAINROOT="/usr"
CMSISROOT="./"
ADIPACKSROOT="./"
```


### Build
```bash
$ cd adicup3029-blinky
$ mkdir build && cd build
$ cmake -DCMAKE_TOOLCHAIN_FILE=../aducm3029.cmake ..
$ make
```

### License

MIT License