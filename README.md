# EECE372 Bare-Metal Programming Examples

Minimal bare-metal examples for the TI MSPM0C1104 (ARM Cortex-M0+), written entirely with direct register access — no DriverLib or SysConfig.

## Set-up Env

Install arm-none-eabi-gcc
- macOS (Homebrew 필요)
```sh
brew install arm-none-eabi-gcc
```
- Windows
Downloads proper installiation file for your computer, and install. (May have to set up Envioment variable settings)
https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads

Install openOCD
배포판은 xds0 지원이 안되어 직접 빌드 필요
-macOS
```sh
git clone https://github.com/openocd-org/openocd.git
cd openocd
git submodule update --init --recursive
./bootstrap
./configure \
  --prefix=$HOME/.local \
  --enable-internal-jimtcl \
  --enable-xds110 \
  --enable-cmsis-dap \
  --enable-ftdi \
  --enable-stlink
make -j
sudo make install
```

## How to Build

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

# Convert to raw binary
arm-none-eabi-objcopy -O binary {name}.elf {name}.bin
```

To flash using OpenOCD:

```sh
openocd -f ../openocd/mspm0c1104_xds110.cfg -c "program {name}.elf verify reset"
```

## mspm0c1104.ld

Linker script that maps the MSPM0C1104 memory layout (16 KB Flash at `0x00000000`, 1 KB SRAM at `0x20000000`) and defines the `.isr_vector`, `.text`, `.data`, and `.bss` sections.

## startup_mspm0c1104.c

Startup file that defines the interrupt vector table and `Reset_Handler`, which copies `.data` from Flash to SRAM, zeros `.bss`, then calls `main()`.

## Examples

- **LED** (`ex_led/`): Blinks an LED on PA22 at ~0.5 Hz using a busy-wait delay loop and bare-metal GPIOA register access.
- **GPIO** (`ex_gpio/`): Toggles the LED on PA22 on each falling-edge press of a button on PA16, handled via a GPIOA interrupt (NVIC IRQ1).
- **DMA** (`ex_dma/`): Performs a software-triggered memory-to-memory DMA transfer (16 words) on channel 0. Turns the LED on solid if the transfer is verified correct, or blinks it rapidly on failure.
