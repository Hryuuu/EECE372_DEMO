# EECE372 Bare-Metal Programming Examples

Minimal bare-metal examples for the TI MSPM0C1104 (ARM Cortex-M0+), written entirely with direct register access — no DriverLib or SysConfig.

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
    startup_mspm0c1104.o main.o -o led.elf

# Convert to raw binary
arm-none-eabi-objcopy -O binary led.elf led.bin
```

To flash using OpenOCD:

```sh
openocd -f ../openocd/mspm0c1104_xds110.cfg -c "program led.elf verify reset exit"
```

## mspm0c1104.ld

Linker script that maps the MSPM0C1104 memory layout (16 KB Flash at `0x00000000`, 1 KB SRAM at `0x20000000`) and defines the `.isr_vector`, `.text`, `.data`, and `.bss` sections.

## startup_mspm0c1104.c

Startup file that defines the interrupt vector table and `Reset_Handler`, which copies `.data` from Flash to SRAM, zeros `.bss`, then calls `main()`.

## Examples

- **LED** (`ex_led/`): Blinks an LED on PA22 at ~0.5 Hz using a busy-wait delay loop and bare-metal GPIOA register access.
- **GPIO** (`ex_gpio/`): Toggles the LED on PA22 on each falling-edge press of a button on PA16, handled via a GPIOA interrupt (NVIC IRQ1).
- **DMA** (`ex_dma/`): Performs a software-triggered memory-to-memory DMA transfer (16 words) on channel 0. Turns the LED on solid if the transfer is verified correct, or blinks it rapidly on failure.
