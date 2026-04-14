# EECE372 Bare-Metal Programming Examples

Minimal bare-metal examples for the TI MSPM0C1104 (ARM Cortex-M0+), written entirely with direct register access and without DriverLib or SysConfig.

Written by Hanseo Ryu  
Contact: hsryu@postech.ac.kr

Some code in this repository was generated with Claude. Any such sections are explicitly marked in the source.

## Overview

This repository contains small, self-contained examples for learning low-level embedded programming on the MSPM0C1104. The focus is on understanding the hardware directly through registers, startup code, linker scripts, interrupts, and memory layout.

## Environment Setup

### Install `arm-none-eabi-gcc`

#### macOS

Homebrew is required.

```sh
brew install arm-none-eabi-gcc
```

#### Windows

Download and install the appropriate package from the official Arm GNU Toolchain page. You may also need to add the toolchain to your `PATH`.

https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads

### Install OpenOCD

Standard OpenOCD distribution packages may not include MSPM0 support. If that is the case on your system, build OpenOCD from source using the instructions below.

#### macOS

```sh
brew install openocd --HEAD
```

#### Windows

Install MSYS2 first, then use the MSYS `UCRT64` terminal:

```sh
pacman -Syu
pacman -S --needed \
  git \
  make \
  mingw-w64-ucrt-x86_64-toolchain \
  mingw-w64-ucrt-x86_64-autotools \
  mingw-w64-ucrt-x86_64-pkgconf \
  mingw-w64-ucrt-x86_64-libusb \
  mingw-w64-ucrt-x86_64-hidapi \
  mingw-w64-ucrt-x86_64-libftdi

git clone https://github.com/openocd-org/openocd.git
cd openocd
git submodule update --init --recursive
./bootstrap
./configure --enable-internal-jimtcl --enable-xds110 --enable-cmsis-dap
make -j"$(nproc)"
make install

cd /c/your/project/folder/
```

## Build

Run the following commands inside one of the example directories, such as `ex_led/`, `ex_gpio/`, or `ex_dma/`.

```sh
# Compile
arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -O0 -ffreestanding -fno-builtin \
    -c startup_mspm0c1104.c -o startup_mspm0c1104.o

arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -O0 -ffreestanding -fno-builtin \
    -c main.c -o main.o

# Link
arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -O0 -ffreestanding -fno-builtin \
    -T mspm0c1104.ld -nostdlib -Wl,--gc-sections \
    startup_mspm0c1104.o main.o -o {name}.elf

# Convert to a raw binary (optional)
arm-none-eabi-objcopy -O binary {name}.elf {name}.bin
```

Build flag summary:

- `-mcpu=cortex-m0plus`: Target the Arm Cortex-M0+ CPU.
- `-mthumb`: Generate Thumb instruction set code.
- `-O0`: Disable optimization for easier debugging.
- `-ffreestanding`: Build for a bare-metal environment without a hosted runtime.
- `-fno-builtin`: Do not replace standard functions with compiler built-ins.
- `-c`: Compile only and produce an object file.
- `-T mspm0c1104.ld`: Use the specified linker script.
- `-nostdlib`: Do not link the standard C library or default startup files.
- `-Wl,--gc-sections`: Remove unused sections during linking.
- `-O binary`: Write the output as a raw binary image.

## Debug

Disassemble the ELF file to inspect the generated machine code and assembly.

```sh
arm-none-eabi-objdump -d {name}.elf
```

## Flash

Use OpenOCD to program the ELF file:

```sh
openocd -f ../openocd/mspm0c1104_xds110.cfg 
```

Open another shell.

### macOS

```sh
nc localhost 4444
program {name}.elf 
verify 
reset
```

### Windows

```sh
telnet localhost 4444
program {name}.elf 
verify 
reset
```


## Repository Structure

### `mspm0c1104.ld`

The linker script defines the MSPM0C1104 memory layout: 16 KB of flash at `0x00000000` and 1 KB of SRAM at `0x20000000`. It also places the `.isr_vector`, `.text`, `.data`, and `.bss` sections.

### `startup_mspm0c1104.c`

The startup file defines the interrupt vector table and `Reset_Handler`. On reset, it copies `.data` from flash to SRAM, clears `.bss`, and then calls `main()`.

Because each example may require different interrupt handlers, check the vector table in the example you are building.

## Examples

- **LED** (`ex_led/`): Blinks an LED on PA22 at approximately 0.5 Hz using a busy-wait delay loop and direct GPIOA register access.
- **GPIO** (`ex_gpio/`): Toggles the LED on PA22 when the button on PA16 is pressed, using a falling-edge GPIOA interrupt (`NVIC IRQ1`).
- **DMA** (`ex_dma/`): Performs a software-triggered memory-to-memory DMA transfer of 16 words on channel 0. The LED remains on if the transfer succeeds and blinks rapidly if verification fails.
- **PC Control** (`ex_pc_con/`): Demonstrates direct program counter control by switching between two LED blink patterns with inline assembly.

## Notes

### Debugging

- OpenOCD can be used for general debugging tasks such as setting breakpoints, examining memory, and stepping through code.
- If OpenOCD does not work as expected, build it directly from the official GitHub source and make sure the submodules are initialized.
- If OpenOCD does not work on your system, you can use Texas Instruments UniFlash as an alternative for programming and device access.

### Document References

- `TRM` in the code refers to:
  Texas Instruments, *MSPM0 C-Series Microcontrollers Technical Reference Manual* (2025)  
  https://www.ti.com/kr/lit/pdf/slau893
- `Datasheet` in the code refers to:
  Texas Instruments, *MSPM0C110x, MSPS003 Mixed-Signal Microcontrollers* (2026)  
  https://www.ti.com/kr/lit/gpn/mspm0c1104

### Additional Resources

- MSPM0 Development Guide  
  https://www.ti.com/kr/lit/pdf/slaaed1
- Arm Cortex-M0+ User Guide  
  https://developer.arm.com/documentation/dui0662/latest/
