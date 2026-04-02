/*
Name: Hanseo Ryu
Date: 2026-4-2
Description: DMA memory-to-memory transfer example on MSPM0C1104.
    Based on TI MSPM0 SDK example (DMA_fill_transfer):
        https://dev.ti.com/tirex/explore/node?isTheia=false&node=A__ALYg04l.OG.SRWm5154FlQ__MSPM0-SDK__a3PaaoK__LATEST
        Converted to bare-metal register access (no DriverLib/SysConfig).
*/
#include <stdint.h>

#define PIN_22_MASK     (1u<<22)

//Base address of peripherals (Datasheet Table 8-5)
#define GPIOA   0x400A0000u
#define IOMUX   0x40428000u
#define DMA     0x4042A000u

//GPIOA registers (TRM Table 10-2)
#define GPIOA_PWREN         *(volatile uint32_t *)(GPIOA+0x800u)
#define GPIOA_DOESET31_0    *(volatile uint32_t *)(GPIOA+0x12D0u)
#define GPIOA_DOUTSET31_0   *(volatile uint32_t *)(GPIOA+0x1290u)
#define GPIOA_DOUTCLR31_0   *(volatile uint32_t *)(GPIOA+0x12A0u)

// IOMUX Base + 0x04(PINCM offset) + (PINCM_index - 1)*4, PIN22-> PINCM23 (Datasheet Table 6-1) 
#define IOMUX_PINCM23       *(volatile uint32_t *)(IOMUX+0x5Cu) 

//ARM Cortex-M0+ Register (TRM Table 3-2)
#define NVIC_ISER           *(volatile uint32_t *)(0xE000E100u)

//DMA channel 0 and interrupt registers (TRM Table 5-8)
#define DMA_IMASK           *(volatile uint32_t *)(DMA+0x1028u)
#define DMA_MIS             *(volatile uint32_t *)(DMA+0x1038u)
#define DMA_ICLR            *(volatile uint32_t *)(DMA+0x1048u)
#define DMA_CH0_DMATCTL     *(volatile uint32_t *)(DMA+0x1110u)
#define DMA_CH0_DMACTL      *(volatile uint32_t *)(DMA+0x1200u)
#define DMA_CH0_DMASA       *(volatile uint32_t *)(DMA+0x1204u)
#define DMA_CH0_DMADA       *(volatile uint32_t *)(DMA+0x1208u)
#define DMA_CH0_DMASZ       *(volatile uint32_t *)(DMA+0x120Cu)

static void MY_init(void) {
    GPIOA_PWREN = 0x26000001u;          //Enable GPIO Power (TRM Table 10-8)
    IOMUX_PINCM23 = 0x00000081u;        //Configure PA22 as GPIO (TRM Table 9-5)
    GPIOA_DOESET31_0 = PIN_22_MASK;     //Enable PA22 as output
    GPIOA_DOUTSET31_0 = PIN_22_MASK;    //Set PA22 output high
    
    DMA_CH0_DMATCTL = 0x00000000u;      // Set DMA CH0 trigger source to software
    DMA_ICLR = 0x00000001u;             //Clear any pending DMA CH0 interrupt flag
    DMA_IMASK = 0x00000001u;            //Enable DMA CH0 transfer-done interrupt
    NVIC_ISER |= (1u<<31);              //Enable NVIC IRQ31
}

static void MY_delayCycles(volatile uint32_t c) {
    while(c-- != 0u) __asm volatile ("nop"); 
}

#define DMA_TRANSFER_SIZE_WORDS (16)

const uint32_t gSrcData[DMA_TRANSFER_SIZE_WORDS] = {0x00000000, 0x10101010,
    0x20202020, 0x30303030, 0x40404040, 0x50505050, 0x60606060, 0x70707070,
    0x80808080, 0x90909090, 0xA0A0A0A0, 0xB0B0B0B0, 0xC0C0C0C0, 0xD0D0D0D0,
    0xE0E0E0E0, 0xF0F0F0F0};
uint32_t gDstData[DMA_TRANSFER_SIZE_WORDS];

volatile uint8_t gChannel0InterruptTaken = 0u;
volatile uint8_t gVerifyResult = 0u;

int main(void) {
    int8_t i;

    MY_init();
    
    DMA_CH0_DMASA = (uint32_t) &gSrcData[0];
    DMA_CH0_DMADA = (uint32_t) &gDstData[0];
    DMA_CH0_DMASZ = sizeof(gSrcData)/sizeof(uint32_t);
    DMA_CH0_DMACTL = 0x10332202u;       //DMA address mode setting (TRM Table 5-30)
    
    gChannel0InterruptTaken = 0u;
    DMA_CH0_DMACTL |= 0x00000001u;      //Start DMA

    while (gChannel0InterruptTaken==0u) {
        __asm volatile ("wfi" ::: "memory");
    }

    gVerifyResult = 1u;
    for (i = 0; i < DMA_TRANSFER_SIZE_WORDS; i++) {
        if (gSrcData[i] != gDstData[i]) gVerifyResult = 0u;
    }
    if(gVerifyResult==1u) { //DMA success
        GPIOA_DOUTCLR31_0 = PIN_22_MASK; //LED on
        while (1) {
        __asm volatile ("wfi" ::: "memory");
        }
    } else {
        while (1) {
            GPIOA_DOUTCLR31_0 = PIN_22_MASK;  //LED on
            MY_delayCycles(1200000);
            GPIOA_DOUTSET31_0 = PIN_22_MASK;  //LED off
            MY_delayCycles(1200000);
        }
    }
}

void DMA_IRQHandler(void) {
    if(DMA_MIS & 0x00000001u) {
        DMA_ICLR = 0x00000001u;
        gChannel0InterruptTaken = 1u;
    }
}