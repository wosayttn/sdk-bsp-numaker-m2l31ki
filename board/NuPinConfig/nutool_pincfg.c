/****************************************************************************
 * @file     nutool_pincfg.c
 * @version  V1.21
 * @Date     2024/4/1-12:06:36
 * @brief    NuMicro generated code file
 *
 * Copyright (C) 2013-2020 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#include "rtconfig.h"
#include "NuMicro.h"

#if defined(BOARD_USING_NUTFT)
void expansion_nutft_pin_init(void)
{
#if defined(BOARD_USING_LCD_ILI9341)
    SYS->GPD_MFP0 &= ~(SYS_GPD_MFP0_PD0MFP_Msk | SYS_GPD_MFP0_PD1MFP_Msk | SYS_GPD_MFP0_PD2MFP_Msk | SYS_GPD_MFP0_PD3MFP_Msk);
    SYS->GPD_MFP0 |= (SYS_GPD_MFP0_PD0MFP_USCI0_CLK | SYS_GPD_MFP0_PD1MFP_USCI0_DAT0 | SYS_GPD_MFP0_PD2MFP_USCI0_DAT1 /*| SYS_GPD_MFP0_PD3MFP_USCI0_CTL1*/);

    SYS->GPA_MFP2 &= ~(SYS_GPA_MFP2_PA8MFP_Msk | SYS_GPA_MFP2_PA9MFP_Msk | SYS_GPA_MFP2_PA11MFP_Msk);
#endif

#if defined(BOARD_USING_NUTFT_LCM_TOUCH)
    GPIO_SetMode(PB, BIT4 | BIT5 | BIT6 | BIT7, GPIO_MODE_INPUT);

    SYS->GPB_MFP1 &= ~(SYS_GPB_MFP1_PB4MFP_Msk | SYS_GPB_MFP1_PB5MFP_Msk | SYS_GPB_MFP1_PB6MFP_Msk | SYS_GPB_MFP1_PB7MFP_Msk);
    SYS->GPB_MFP1 |= (SYS_GPB_MFP1_PB4MFP_EADC0_CH4 | SYS_GPB_MFP1_PB4MFP_EADC0_CH5 | SYS_GPB_MFP1_PB4MFP_EADC0_CH6 | SYS_GPB_MFP1_PB4MFP_EADC0_CH7);

    /* Disable digital path on these ADC pins */
    GPIO_DISABLE_DIGITAL_PATH(PB, BIT4 | BIT5 | BIT6 | BIT7);
#endif

#if defined(BOARD_USING_NUTFT_QSPI_FLASH)
    SYS->GPA_MFP0 &= ~(SYS_GPA_MFP0_PA0MFP_Msk | SYS_GPA_MFP0_PA1MFP_Msk | SYS_GPA_MFP0_PA2MFP_Msk | SYS_GPA_MFP0_PA3MFP_Msk);
    SYS->GPA_MFP0 |= (SYS_GPA_MFP0_PA0MFP_QSPI0_MOSI0 | SYS_GPA_MFP0_PA1MFP_QSPI0_MISO0 | SYS_GPA_MFP0_PA2MFP_QSPI0_CLK | SYS_GPA_MFP0_PA3MFP_QSPI0_SS);

    SYS->GPB_MFP0 &= ~(SYS_GPB_MFP0_PB0MFP_Msk | SYS_GPB_MFP0_PB1MFP_Msk);
    SYS->GPB_MFP0 |= (SYS_GPB_MFP0_PB0MFP_QSPI0_MOSI1 | SYS_GPB_MFP0_PB1MFP_QSPI0_MISO1);
#endif

#if defined(BOARD_USING_NUTFT_BUTTON)
    SYS->GPB_MFP0 &= ~(SYS_GPB_MFP0_PB2MFP_Msk | SYS_GPB_MFP0_PB3MFP_Msk);

    /* Disable digital path on these ADC pins */
    GPIO_ENABLE_DIGITAL_PATH(PB, BIT2 | BIT3);
#endif

}
#endif

void nutool_pincfg_init_uart0(void)
{
    SYS->GPB_MFP3 &= ~(SYS_GPB_MFP3_PB12MFP_Msk | SYS_GPB_MFP3_PB13MFP_Msk);
    SYS->GPB_MFP3 |= (SYS_GPB_MFP3_PB13MFP_UART0_TXD | SYS_GPB_MFP3_PB12MFP_UART0_RXD);
}

void nutool_pincfg_init_usb(void)
{
    /* USB_VBUS_EN (USB 1.1 VBUS power enable pin) multi-function pin - PB.15 */
    SYS->GPB_MFP3 = (SYS->GPB_MFP3 & ~SYS_GPB_MFP3_PB15MFP_Msk) | SYS_GPB_MFP3_PB15MFP_USB_VBUS_EN;

    /* USB_VBUS_ST (USB 1.1 over-current detect pin) multi-function pin - PC.14 */
    //SYS->GPC_MFP3 = (SYS->GPC_MFP3 & ~SYS_GPC_MFP3_PC14MFP_Msk) | SYS_GPC_MFP3_PC14MFP_USB_VBUS_ST;

    /*  USB 1.1 port multi-function pin VBUS, D+, D-, and OTG_ID pins:
        PA12: USB_VBUS_DT
        PA13: USB_D-
        PA14: USB_D+
        PA15: PB15_VBUS_SEN  */
    SYS->GPA_MFP3 &= ~(SYS_GPA_MFP3_PA12MFP_Msk | SYS_GPA_MFP3_PA13MFP_Msk |
                       SYS_GPA_MFP3_PA14MFP_Msk /*| SYS_GPA_MFP3_PA15MFP_Msk */);

    SYS->GPA_MFP3 |= SYS_GPA_MFP3_PA12MFP_USB_VBUS | SYS_GPA_MFP3_PA13MFP_USB_D_N |
                     SYS_GPA_MFP3_PA14MFP_USB_D_P /*| SYS_GPA_MFP3_PA15MFP_USB_OTG_ID */;
}

void nutool_pincfg_init(void)
{
#if defined(BSP_USING_UART0)
    nutool_pincfg_init_uart0();
#endif

#if defined(BSP_USING_USBD) || defined(BSP_USING_USBH)
    nutool_pincfg_init_usb();
#endif

#if defined(BOARD_USING_NUTFT)
    expansion_nutft_pin_init();
#endif

    return;
}

void nutool_pincfg_deinit(void)
{
    return;
}
