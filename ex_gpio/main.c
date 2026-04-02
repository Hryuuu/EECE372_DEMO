/*
Name: Hanseo Ryu
Date: 2026-4-2
Description: Toggle LED on PA22 via GPIO interrupt triggered by button press.
*/
#include <stdint.h>

#define PIN_22_MASK     (1u<<22)
#define PIN_16_MASK     (1u<<16)

//Base address of peripherals (Datasheet Table 8-5)
#define GPIOA   0x400A0000u
#define IOMUX   0x40428000u

//GPIOA registers (TRM Table 10-2)
#define GPIOA_PWREN         *(volatile uint32_t *)(GPIOA+0x800u)
#define GPIOA_DOESET31_0    *(volatile uint32_t *)(GPIOA+0x12D0u)
#define GPIOA_DOUTSET31_0   *(volatile uint32_t *)(GPIOA+0x1290u)
#define GPIOA_DOUTCLR31_0   *(volatile uint32_t *)(GPIOA+0x12A0u)
#define GPIOA_IMASK         *(volatile uint32_t *)(GPIOA+0x1028u)
#define GPIOA_MIS           *(volatile uint32_t *)(GPIOA+0x1038u)
#define GPIOA_ICLR          *(volatile uint32_t *)(GPIOA+0x1048u)
#define GPIOA_POLARITY31_16 *(volatile uint32_t *)(GPIOA+0x13a0u)

// IOMUX Base + 0x04(PINCM offset) + (PINCM_index - 1)*4, PIN22-> PINCM23 (Datasheet Table 6-1) 
#define IOMUX_PINCM23       *(volatile uint32_t *)(IOMUX+0x5Cu) 
#define IOMUX_PINCM17       *(volatile uint32_t *)(IOMUX+0x44u)

//ARM Cortex-M0+ Register (TRM Table 3-2)
#define NVIC_ISER           *(volatile uint32_t *)(0xE000E100u)

static volatile uint8_t led_on=0u;

static void MY_init(void) {
    GPIOA_PWREN = 0x26000001u;          //Enable GPIO Power (TRM Table 10-8)
    //Init LED pin
    IOMUX_PINCM23 = 0x00000081u;        //Configure PA22 as GPIO - PC, PF (TRM Table 9-5)
    GPIOA_DOESET31_0 = PIN_22_MASK;     //Enable PA22 as output
    GPIOA_DOUTSET31_0 = PIN_22_MASK;    //Set PA22 output high
    //Init User Button
    IOMUX_PINCM17 = 0x00060081u;        //Configure PA16 - INENA, Pullup, PC, PF (TRM Table 9-5)
    GPIOA_POLARITY31_16 &= ~0x00000003u;//Clear polarity field for DIO16
    GPIOA_POLARITY31_16 |=  0x00000002u;//Set falling edge trigger for DIO16
    GPIOA_ICLR = PIN_16_MASK;           //Clear any pending flag
    GPIOA_IMASK |= PIN_16_MASK;         //Enable interrupt
    NVIC_ISER |= (1u<<1);               //Enable NVIC IRQ1
}

int main(void) {

    MY_init();
    
    while (1) {
        __asm volatile ("wfi":::"memory");  //Wait for interrupt
    }
}

void GPIOA_IRQHandler(void) {
    if(GPIOA_MIS & PIN_16_MASK) {
        GPIOA_ICLR = PIN_16_MASK;  //Clear PA16 interrupt flag
        if(led_on) {
            GPIOA_DOUTSET31_0 = PIN_22_MASK; //LED off
            led_on=0u;
        } else {
            GPIOA_DOUTCLR31_0 = PIN_22_MASK; //LED on
            led_on=1u;
        }

    }
}