/**************************************************************************//**
 * @file     startup_M2L31.c
 * @version  V1.00
 * @brief    CMSIS Device Startup File for NuMicro M55M1
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2023 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/

#include "rtconfig.h"
#include <inttypes.h>
#include <stdio.h>
#include "NuMicro.h"

/*----------------------------------------------------------------------------
  External References
 *----------------------------------------------------------------------------*/
extern uint32_t __INITIAL_SP;
extern uint32_t __STACK_LIMIT;

extern __NO_RETURN void __PROGRAM_START(void);

/*----------------------------------------------------------------------------
  Internal References
 *----------------------------------------------------------------------------*/
__NO_RETURN void Reset_Handler(void);
void Default_Handler(void);

/*----------------------------------------------------------------------------
  Exception / Interrupt Handler
 *----------------------------------------------------------------------------*/
/* Exceptions */
void NMI_Handler(void) __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler(void) __attribute__((weak));
void MemManage_Handler(void) __attribute__((weak, alias("Default_Handler")));
void BusFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void UsageFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SVC_Handler(void) __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler(void) __attribute__((weak, alias("Default_Handler")));

/* External Interrupts */
void BOD_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));      // 0: Brown Out detection
void IRC_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));      // 1: Internal RC
void PWRWU_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));    // 2: Power down wake up
void RAMPE_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));    // 3: RAM parity error
void CLKFAIL_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));  // 4: Clock detection fail
void RRMC_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));     // 5: RRMC (ISP)
void RTC_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));      // 6: Real Time Clock
void TAMPER_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));   // 7: Tamper interrupt
void WDT_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));      // 8: Watchdog timer
void WWDT_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));     // 9: Window watchdog timer
void EINT0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));    // 10: External Input 0
void EINT1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));    // 11: External Input 1
void EINT2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));    // 12: External Input 2
void EINT3_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));    // 13: External Input 3
void EINT4_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));    // 14: External Input 4
void EINT5_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));    // 15: External Input 5
void GPA_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));      // 16: GPIO Port A
void GPB_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));      // 17: GPIO Port B
void GPC_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));      // 18: GPIO Port C
void GPD_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));      // 19: GPIO Port D
void GPE_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));      // 20: GPIO Port E
void GPF_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));      // 21: GPIO Port F
void QSPI0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));    // 22: QSPI0
void SPI0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));     // 23: SPI0
void EBRAKE0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));  // 24:
void EPWM0P0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));  // 25:
void EPWM0P1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));  // 26:
void EPWM0P2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));  // 27:
void EBRAKE1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));  // 28:
void EPWM1P0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));  // 29:
void EPWM1P1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));  // 30:
void EPWM1P2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));  // 31:
void TMR0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));     // 32: Timer 0
void TMR1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));     // 33: Timer 1
void TMR2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));     // 34: Timer 2
void TMR3_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));     // 35: Timer 3
void UART0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));    // 36: UART0
void UART1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));    // 37: UART1
void I2C0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));     // 38: I2C0
void I2C1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));     // 39: I2C1
void PDMA0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));    // 40: Peripheral DMA 0
void DAC_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));      // 41: DAC
void EADC0_INT0_IRQHandler(void) __attribute__((weak, alias("Default_Handler"))); // 42: EADC0 interrupt source 0
void EADC0_INT1_IRQHandler(void) __attribute__((weak, alias("Default_Handler"))); // 43: EADC0 interrupt source 1
void ACMP01_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));   // 44: ACMP0 and ACMP1
void ACMP2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));    // 45: ACMP2
void EADC0_INT2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));  // 46: EADC0 interrupt source 2
void EADC0_INT3_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));  // 47: EADC0 interrupt source 3
void UART2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));    // 48: UART2
void UART3_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));    // 49: UART3
void SPI1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));     // 51: SPI1
void SPI2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));     // 52: SPI2
void USBD_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));     // 53: USB device
void USBH_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));     // 54: USB host
void USBOTG_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));   // 55: USB OTG
void ETI_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));      // 56: ETI
void CRC0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));     // 57: CRC0
void SPI3_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));     // 62: SPI3
void TK_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));       // 63: Touch Key
void OPA012_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));   // 70: OPA012
void CRPT_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));     // 71: CRPT
void GPG_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));      // 72: GPIO Port G
void EINT6_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));    // 73: External Input 6
void UART4_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));    // 74: UART4
void UART5_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));    // 75: UART5
void USCI0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));    // 76: USCI0
void USCI1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));    // 77: USCI1
void I2C2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));     // 82: I2C2
void I2C3_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));     // 83: I2C3
void EQEI0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));    // 84: EQEI0
void EQEI1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));    // 85: EQEI1
void ECAP0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));    // 86: ECAP0
void ECAP1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));    // 87: ECAP1
void GPH_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));      // 88: GPIO Port H
void EINT7_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));    // 89: External Input 7
void LPPDMA0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));  // 98: LPPDMA0
void TRNG_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));     // 101: TRNG
void UART6_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));    // 102: UART6
void UART7_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));    // 103: UART7
void UTCPD_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));    // 108: UTCPD
void CANFD00_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));  // 112: CAN FD 00
void CANFD01_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));  // 113: CAN FD 01
void CANFD10_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));  // 114: CAN FD 10
void CANFD11_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));  // 115: CAN FD 11
void BRAKE0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));   // 128: BRAKE0
void PWM0P0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));   // 129: PWM0P0
void PWM0P1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));   // 130: PWM0P1
void PWM0P2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));   // 131: PWM0P2
void BRAKE1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));   // 132: BRAKE1
void PWM1P0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));   // 133: PWM1P0
void PWM1P1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));   // 134: PWM1P1
void PWM1P2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));   // 135: PWM1P2
void LPADC0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));   // 136: LPADC0
void LPUART0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));  // 137: LPUART0
void LPI2C0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));   // 138: LPI2C0
void LPSPI0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));   // 139: LPSPI0
void LPTMR0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));   // 140: LPTMR0
void LPTMR1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));   // 141: LPTMR1
void TTMR0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));    // 142: TTMR0
void TTMR1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));    // 143: TTMR1

/*----------------------------------------------------------------------------
  Exception / Interrupt Vector table
 *----------------------------------------------------------------------------*/
#if defined ( __GNUC__ )
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wpedantic"
#endif

/* Static vector table
 * For performance, M55M1 places vector table in DTCM by default.
 * User can define NVT_VECTOR_ON_FLASH to place vector table in Flash.
 *
 * If NVT_VECTOR_ON_FLASH is defined and use IAR,
 *   IRQ handlers referenced in __VECTOR_TABLE (__vector_table) are protected and
 *   not affected by 'initialize by copy'. It means IRQ handler must placed in Flash.
 */
const VECTOR_TABLE_Type __VECTOR_TABLE[] __VECTOR_TABLE_ATTRIBUTE =
{
    (VECTOR_TABLE_Type)(&__INITIAL_SP),       /*       Initial Stack Pointer                            */
    Reset_Handler,                            /*       Reset Handler                                    */
    NMI_Handler,                              /*   -14 NMI Handler                                      */
    HardFault_Handler,                        /*   -13 Hard Fault Handler                               */
    MemManage_Handler,                        /*   -12 MPU Fault Handler                                */
    BusFault_Handler,                         /*   -11 Bus Fault Handler                                */
    UsageFault_Handler,                       /*   -10 Usage Fault Handler                              */
    0,                                        /*    -9 Reserved                                         */
    0,                                        /*    -8 Reserved                                         */
    0,                                        /*    -7 Reserved                                         */
    0,                                        /*    -6 Reserved                                         */
    SVC_Handler,                              /*    -5 SVC Handler                                      */
    0,                                        /*    -4 Reserved                                         */
    0,                                        /*    -3 Reserved                                         */
    PendSV_Handler,                           /*    -2 PendSV Handler Handler                           */
    SysTick_Handler,                          /*    -1 SysTick Handler                                  */

    /* Interrupts */
    BOD_IRQHandler,                           /*    0: Brown Out detection                               */
    IRC_IRQHandler,                           /*    1: Internal RC                                       */
    PWRWU_IRQHandler,                         /*    2: Power down wake up                                */
    RAMPE_IRQHandler,                         /*    3: RAM parity error                                  */
    CLKFAIL_IRQHandler,                       /*    4: Clock detection fail                              */
    RRMC_IRQHandler,                          /*    5: RRMC (ISP)                                        */
    RTC_IRQHandler,                           /*    6: Real Time Clock                                   */
    TAMPER_IRQHandler,                        /*    7: Tamper interrupt                                  */
    WDT_IRQHandler,                           /*    8: Watchdog timer                                    */
    WWDT_IRQHandler,                          /*    9: Window watchdog timer                             */
    EINT0_IRQHandler,                         /*    10: External Input 0                                 */
    EINT1_IRQHandler,                         /*    11: External Input 1                                 */
    EINT2_IRQHandler,                         /*    12: External Input 2                                 */
    EINT3_IRQHandler,                         /*    13: External Input 3                                 */
    EINT4_IRQHandler,                         /*    14: External Input 4                                 */
    EINT5_IRQHandler,                         /*    15: External Input 5                                 */
    GPA_IRQHandler,                           /*    16: GPIO Port A                                      */
    GPB_IRQHandler,                           /*    17: GPIO Port B                                      */
    GPC_IRQHandler,                           /*    18: GPIO Port C                                      */
    GPD_IRQHandler,                           /*    19: GPIO Port D                                      */
    GPE_IRQHandler,                           /*    20: GPIO Port E                                      */
    GPF_IRQHandler,                           /*    21: GPIO Port F                                      */
    QSPI0_IRQHandler,                         /*    22: QSPI0                                            */
    SPI0_IRQHandler,                          /*    23: SPI0                                             */
    EBRAKE0_IRQHandler,                       /*    24: EBRAKE0                                          */
    EPWM0P0_IRQHandler,                       /*    25: EPWM0P0                                          */
    EPWM0P1_IRQHandler,                       /*    26: EPWM0P1                                          */
    EPWM0P2_IRQHandler,                       /*    27: EPWM0P2                                          */
    EBRAKE1_IRQHandler,                       /*    28: EBRAKE1                                          */
    EPWM1P0_IRQHandler,                       /*    29: EPWM1P0                                          */
    EPWM1P1_IRQHandler,                       /*    30: EPWM1P1                                          */
    EPWM1P2_IRQHandler,                       /*    31: EPWM1P2                                          */
    TMR0_IRQHandler,                          /*    32: Timer 0                                          */
    TMR1_IRQHandler,                          /*    33: Timer 1                                          */
    TMR2_IRQHandler,                          /*    34: Timer 2                                          */
    TMR3_IRQHandler,                          /*    35: Timer 3                                          */
    UART0_IRQHandler,                         /*    36: UART0                                            */
    UART1_IRQHandler,                         /*    37: UART1                                            */
    I2C0_IRQHandler,                          /*    38: I2C0                                             */
    I2C1_IRQHandler,                          /*    39: I2C1                                             */
    PDMA0_IRQHandler,                         /*    40: Peripheral DMA 0                                 */
    DAC_IRQHandler,                           /*    41: DAC                                              */
    EADC0_INT0_IRQHandler,                    /*    42: EADC0 interrupt source 0                         */
    EADC0_INT1_IRQHandler,                    /*    43: EADC0 interrupt source 1                         */
    ACMP01_IRQHandler,                        /*    44: ACMP0 and ACMP1                                  */
    ACMP2_IRQHandler,                         /*    45: ACMP2                                            */
    EADC0_INT2_IRQHandler,                    /*    46: EADC0 interrupt source 2                         */
    EADC0_INT3_IRQHandler,                    /*    47: EADC0 interrupt source 3                         */
    UART2_IRQHandler,                         /*    48: UART2                                            */
    UART3_IRQHandler,                         /*    49: UART3                                            */
    0,                                        /*    50: Reserved                                         */
    SPI1_IRQHandler,                          /*    51: SPI1                                             */
    SPI2_IRQHandler,                          /*    52: SPI2                                             */
    USBD_IRQHandler,                          /*    53: USB device                                       */
    USBH_IRQHandler,                          /*    54: USB host                                         */
    USBOTG_IRQHandler,                        /*    55: USB OTG                                          */
    ETI_IRQHandler,                           /*    56: ETI                                              */
    CRC0_IRQHandler,                          /*    57: CRC0                                             */
    0,                                        /*    58: Reserved                                         */
    0,                                        /*    59: Reserved                                         */
    0,                                        /*    60: Reserved                                         */
    0,                                        /*    61: Reserved                                         */
    SPI3_IRQHandler,                          /*    62: SPI3                                             */
    TK_IRQHandler,                            /*    63: Touch Key                                        */
    0,                                        /*    64: Reserved                                         */
    0,                                        /*    65: Reserved                                         */
    0,                                        /*    66: Reserved                                         */
    0,                                        /*    67: Reserved                                         */
    0,                                        /*    68: Reserved                                         */
    0,                                        /*    69: Reserved                                         */
    OPA012_IRQHandler,                        /*    70: OPA012                                           */
    CRPT_IRQHandler,                          /*    71: CRPT                                             */
    GPG_IRQHandler,                           /*    72: GPIO Port G                                      */
    EINT6_IRQHandler,                         /*    73: External Input 6                                 */
    UART4_IRQHandler,                         /*    74: UART4                                            */
    UART5_IRQHandler,                         /*    75: UART5                                            */
    USCI0_IRQHandler,                         /*    76: USCI0                                            */
    USCI1_IRQHandler,                         /*    77: USCI1                                            */
    0,                                        /*    78: Reserved                                         */
    0,                                        /*    79: Reserved                                         */
    0,                                        /*    80: Reserved                                         */
    0,                                        /*    81: Reserved                                         */
    I2C2_IRQHandler,                          /*    82: I2C2                                             */
    I2C3_IRQHandler,                          /*    83: I2C3                                             */
    EQEI0_IRQHandler,                         /*    84: EQEI0                                            */
    EQEI1_IRQHandler,                         /*    85: EQEI1                                            */
    ECAP0_IRQHandler,                         /*    86: ECAP0                                            */
    ECAP1_IRQHandler,                         /*    87: ECAP1                                            */
    GPH_IRQHandler,                           /*    88: GPIO Port H                                      */
    EINT7_IRQHandler,                         /*    89: External Input 7                                 */
    0,                                        /*    90: Reserved                                         */
    0,                                        /*    91: Reserved                                         */
    0,                                        /*    92: Reserved                                         */
    0,                                        /*    93: Reserved                                         */
    0,                                        /*    94: Reserved                                         */
    0,                                        /*    95: Reserved                                         */
    0,                                        /*    96: Reserved                                         */
    0,                                        /*    97: Reserved                                         */
    LPPDMA0_IRQHandler,                       /*    98: LPPDMA0                                          */
    0,                                        /*    99: Reserved                                         */
    0,                                        /*    100: Reserved                                        */
    TRNG_IRQHandler,                          /*    101: TRNG                                            */
    UART6_IRQHandler,                         /*    102: UART6                                           */
    UART7_IRQHandler,                         /*    103: UART7                                           */
    0,                                        /*    104: Reserved                                        */
    0,                                        /*    105: Reserved                                        */
    0,                                        /*    106: Reserved                                        */
    0,                                        /*    107: Reserved                                        */
    UTCPD_IRQHandler,                         /*    108: UTCPD                                           */
    0,                                        /*    109: Reserved                                        */
    0,                                        /*    110: Reserved                                        */
    0,                                        /*    110: Reserved                                        */
    CANFD00_IRQHandler,                       /*    112: CAN FD 00                                       */
    CANFD01_IRQHandler,                       /*    113: CAN FD 01                                       */
    CANFD10_IRQHandler,                       /*    114: CAN FD 10                                       */
    CANFD11_IRQHandler,                       /*    115: CAN FD 11                                       */
    0,                                        /*    116: Reserved                                        */
    0,                                        /*    117: Reserved                                        */
    0,                                        /*    118: Reserved                                        */
    0,                                        /*    119: Reserved                                        */
    0,                                        /*    120: Reserved                                        */
    0,                                        /*    121: Reserved                                        */
    0,                                        /*    122: Reserved                                        */
    0,                                        /*    123: Reserved                                        */
    0,                                        /*    124: Reserved                                        */
    0,                                        /*    125: Reserved                                        */
    0,                                        /*    126: Reserved                                        */
    0,                                        /*    127: Reserved                                        */
    BRAKE0_IRQHandler,                        /*    128: BRAKE0                                          */
    PWM0P0_IRQHandler,                        /*    129: PWM0P0                                          */
    PWM0P1_IRQHandler,                        /*    130: PWM0P1                                          */
    PWM0P2_IRQHandler,                        /*    131: PWM0P2                                          */
    BRAKE1_IRQHandler,                        /*    132: BRAKE1                                          */
    PWM1P0_IRQHandler,                        /*    133: PWM1P0                                          */
    PWM1P1_IRQHandler,                        /*    134: PWM1P1                                          */
    PWM1P2_IRQHandler,                        /*    135: PWM1P2                                          */
    LPADC0_IRQHandler,                        /*    136: LPADC0                                          */
    LPUART0_IRQHandler,                       /*    137: LPUART0                                         */
    LPI2C0_IRQHandler,                        /*    138: LPI2C0                                          */
    LPSPI0_IRQHandler,                        /*    139: LPSPI0                                          */
    LPTMR0_IRQHandler,                        /*    140: LPTMR0                                          */
    LPTMR1_IRQHandler,                        /*    141: LPTMR1                                          */
    TTMR0_IRQHandler,                         /*    142: TTMR0                                           */
    TTMR1_IRQHandler                          /*    143: TTMR1                                           */
};

#if defined ( __GNUC__ )
    #pragma GCC diagnostic pop
#endif

__WEAK void Reset_Handler_PreInit(void)
{
    // Empty function
}

/*----------------------------------------------------------------------------
  Reset Handler called on controller reset
 *----------------------------------------------------------------------------*/
__NO_RETURN void Reset_Handler(void)
{
    // Copy __set_PSP/__set_MSPLIM/__set_PSPLIM from cmsis_armclang.h
    __ASM volatile("MSR psp, %0" : : "r"((uint32_t)(&__INITIAL_SP)) :);
    __ASM volatile("MSR msplim, %0" : : "r"((uint32_t)(&__STACK_LIMIT)));
    __ASM volatile("MSR psplim, %0" : : "r"((uint32_t)(&__STACK_LIMIT)));

#if defined(BSP_USING_CUSTOM_LOADER)
    uint32_t custom_loader_exec_last(void);
    if (custom_loader_exec_last())
    {
        __ASM volatile("MSR psp, %0" : : "r"((uint32_t)(&__INITIAL_SP)) :);
        __ASM volatile("MSR msplim, %0" : : "r"((uint32_t)(&__STACK_LIMIT)));
        __ASM volatile("MSR psplim, %0" : : "r"((uint32_t)(&__STACK_LIMIT)));
    }
#endif

    Reset_Handler_PreInit();

    __PROGRAM_START();      // Enter PreMain (C library entry point)
}

#if defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wmissing-noreturn"
#endif

/*----------------------------------------------------------------------------
  Default Handler for Exceptions / Interrupts
 *----------------------------------------------------------------------------*/
void Default_Handler(void)
{
    while (1);
}

#if defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
    #pragma clang diagnostic pop
#endif
