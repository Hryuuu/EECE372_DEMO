/*
Name: Hanseo Ryu
Date: 2026-4-2
Description: Blink LED on PA22 using bare-metal GPIO register access on MSPM0C1104
*/
#include <stdint.h>

#define PIN_22_MASK     (1u<<22)

//Base address of peripherals (Datasheet Table 8-5)
#define GPIOA   0x400A0000u
#define IOMUX   0x40428000u

//GPIOA registers (TRM Table 10-2)
#define GPIOA_PWREN         *(volatile uint32_t *)(GPIOA+0x800u)
#define GPIOA_DOESET31_0    *(volatile uint32_t *)(GPIOA+0x12D0u)
#define GPIOA_DOUTSET31_0   *(volatile uint32_t *)(GPIOA+0x1290u)
#define GPIOA_DOUTCLR31_0   *(volatile uint32_t *)(GPIOA+0x12A0u)

// IOMUX Base + 0x04(PINCM offset) + (PINCM_index - 1)*4, PIN22-> PINCM23 (Datasheet Table 6-1) 
#define IOMUX_PINCM23       *(volatile uint32_t *)(IOMUX+0x5Cu) 

static void MY_init(void) {
    GPIOA_PWREN = 0x26000001u;          //Enable GPIO Power (TRM Table 10-8)
    IOMUX_PINCM23 = 0x00000081u;        //Configure PA22 as GPIO (TRM Table 9-5)
    GPIOA_DOESET31_0 = PIN_22_MASK;     //Enable PA22 as output
    GPIOA_DOUTSET31_0 = PIN_22_MASK;    //Set PA22 output high
}

static void MY_delayCycles(volatile uint32_t c) {
    while(c-- != 0u) __asm volatile ("nop"); 
}

int main(void) {

    MY_init();
    
    while (1) {
        GPIOA_DOUTCLR31_0 = PIN_22_MASK;  //LED on
        MY_delayCycles(2400000);
        GPIOA_DOUTSET31_0 = PIN_22_MASK;  //LED off
        MY_delayCycles(2400000);
    }
}