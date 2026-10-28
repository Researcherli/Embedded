/*!
    \file    gd32f4xx_it.c
    \brief   interrupt service routines

    \version 2024-12-20, V3.3.1, firmware for GD32F4xx
*/

#include "gd32f4xx_it.h"
#include "systick.h"
#include "FreeRTOS.h"
#include "task.h"
#include "usb_driver.h"
#include "drv_usbd_int.h"

typedef struct {
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r12;
    uint32_t lr;
    uint32_t pc;
    uint32_t psr;
    uint32_t cfsr;
    uint32_t hfsr;
    uint32_t dfsr;
    uint32_t afsr;
    uint32_t mmfar;
    uint32_t bfar;
    uint32_t fault_type;
} FaultInfo_t;

static volatile FaultInfo_t __attribute__((section(".noinit"))) g_fault_info;
static volatile uint32_t __attribute__((section(".noinit"))) g_fault_occurred;

#define FAULT_MAGIC  0xDEADBEEF

/*
 * Disable automatic reset after faults by default.
 * This avoids a reset loop and keeps the fault context available for debugging.
 */
#ifndef FAULT_AUTO_RESET_ENABLE
#define FAULT_AUTO_RESET_ENABLE 0
#endif

uint8_t fault_check_occurred(void)
{
    return (g_fault_occurred == FAULT_MAGIC) ? 1U : 0U;
}

const FaultInfo_t *fault_get_info(void)
{
    return (const FaultInfo_t *)&g_fault_info;
}

void fault_clear(void)
{
    g_fault_occurred = 0U;
}

static void system_soft_reset(void)
{
    __DSB();
    SCB->AIRCR = (0x5FAUL << SCB_AIRCR_VECTKEY_Pos) | SCB_AIRCR_SYSRESETREQ_Msk;
    __DSB();

    while (1) {
    }
}

static void fault_halt(void)
{
    __disable_irq();
    __DSB();
    __ISB();

	  NVIC_SystemReset();
    while (1) {
#if defined(DEBUG)
        __BKPT(0);
#endif
    }
}

void fault_handler_c(uint32_t *stack_frame, uint32_t fault_type)
{
    g_fault_info.r0 = stack_frame[0];
    g_fault_info.r1 = stack_frame[1];
    g_fault_info.r2 = stack_frame[2];
    g_fault_info.r3 = stack_frame[3];
    g_fault_info.r12 = stack_frame[4];
    g_fault_info.lr = stack_frame[5];
    g_fault_info.pc = stack_frame[6];
    g_fault_info.psr = stack_frame[7];
    g_fault_info.cfsr = SCB->CFSR;
    g_fault_info.hfsr = SCB->HFSR;
    g_fault_info.dfsr = SCB->DFSR;
    g_fault_info.afsr = SCB->AFSR;
    g_fault_info.mmfar = SCB->MMFAR;
    g_fault_info.bfar = SCB->BFAR;
    g_fault_info.fault_type = fault_type;
    g_fault_occurred = FAULT_MAGIC;

#if FAULT_AUTO_RESET_ENABLE
    system_soft_reset();
#else
    fault_halt();
#endif
}

void NMI_Handler(void)
{
    fault_halt();
}

void __attribute__((naked)) HardFault_Handler(void)
{
    __asm volatile(
        "TST LR, #4         \n"
        "ITE EQ             \n"
        "MRSEQ R0, MSP      \n"
        "MRSNE R0, PSP      \n"
        "MOV R1, #0         \n"
        "B fault_handler_c  \n");
}

void __attribute__((naked)) MemManage_Handler(void)
{
    __asm volatile(
        "TST LR, #4         \n"
        "ITE EQ             \n"
        "MRSEQ R0, MSP      \n"
        "MRSNE R0, PSP      \n"
        "MOV R1, #1         \n"
        "B fault_handler_c  \n");
}

void __attribute__((naked)) BusFault_Handler(void)
{
    __asm volatile(
        "TST LR, #4         \n"
        "ITE EQ             \n"
        "MRSEQ R0, MSP      \n"
        "MRSNE R0, PSP      \n"
        "MOV R1, #2         \n"
        "B fault_handler_c  \n");
}

void __attribute__((naked)) UsageFault_Handler(void)
{
    __asm volatile(
        "TST LR, #4         \n"
        "ITE EQ             \n"
        "MRSEQ R0, MSP      \n"
        "MRSNE R0, PSP      \n"
        "MOV R1, #3         \n"
        "B fault_handler_c  \n");
}

void DebugMon_Handler(void)
{
    fault_halt();
}

void USBFS_IRQHandler(void)
{
    usb_core_driver *usb = usb_cdc_acm();

    if (usb == NULL) {
        return;
    }

    usbd_isr(usb);
}
