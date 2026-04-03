# EECE372 Bare-Metal Programming Examples

Minimal bare-metal examples for the TI MSPM0C1104 (ARM Cortex-M0+), written entirely with direct register access without DriverLib or SysConfig.

Written by Hanseo Ryu.

Contact: hsryu@postech.ac.kr

- Some code was generated with Claude.
- In such cases, it is explicitly marked in the code.

## Environment Setup

### Install `arm-none-eabi-gcc`

#### macOS

Requires Homebrew:

```sh
brew install arm-none-eabi-gcc
```

#### Windows

Download the appropriate installer for your system from the official Arm GNU Toolchain page, then complete the installation. You may also need to configure your `PATH` environment variable.

https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads

### Install OpenOCD

#### macOS

```sh
brew install openocd --HEAD
```

#### Windows

Follow the official installation guide. You may need to install Node.js and npm first, and you may also need to configure your environment variables.

https://xpack-dev-tools.github.io/openocd-xpack/docs/install/

## How to Build

Run the following commands inside one of the example directories such as `ex_led/`, `ex_gpio/`, or `ex_dma/`.

```sh
# Compile
arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -O2 -ffreestanding -fno-builtin \
    -Wall -Wextra -Werror -c startup_mspm0c1104.c -o startup_mspm0c1104.o

arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -O2 -ffreestanding -fno-builtin \
    -Wall -Wextra -Werror -c main.c -o main.o

# Link
arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -O2 -ffreestanding -fno-builtin \
    -T mspm0c1104.ld -nostdlib -Wl,--gc-sections \
    startup_mspm0c1104.o main.o -o {name}.elf

# Convert to a raw binary
arm-none-eabi-objcopy -O binary {name}.elf {name}.bin
```

To flash the program with OpenOCD:

```sh
openocd -f ../openocd/mspm0c1104_xds110.cfg -c "program {name}.elf verify reset"
```

## File Overview

### `mspm0c1104.ld`

This linker script defines the MSPM0C1104 memory layout: 16 KB of Flash at `0x00000000` and 1 KB of SRAM at `0x20000000`. It also places the `.isr_vector`, `.text`, `.data`, and `.bss` sections.

### `startup_mspm0c1104.c`

This startup file defines the interrupt vector table and `Reset_Handler`. On reset, it copies `.data` from Flash to SRAM, clears `.bss`, and then calls `main()`. Check the interrupt vector table for each example, because the required entries may differ depending on the example.

## Examples

- **LED** (`ex_led/`): Blinks an LED on PA22 at approximately 0.5 Hz using a busy-wait delay loop and direct GPIOA register access.
- **GPIO** (`ex_gpio/`): Toggles the LED on PA22 whenever the button on PA16 is pressed on a falling edge, using a GPIOA interrupt (NVIC IRQ1).
- **DMA** (`ex_dma/`): Performs a software-triggered memory-to-memory DMA transfer of 16 words on channel 0. The LED stays on if the transfer succeeds and blinks rapidly if verification fails.
