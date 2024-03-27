/**************************************************************************//**
*
* @copyright (C) 2024 Nuvoton Technology Corp. All rights reserved.
*
* SPDX-License-Identifier: Apache-2.0
*
* Change Logs:
* Date            Author       Notes
* 2024-4-1        Wayne        First version
*
******************************************************************************/

#include <rtconfig.h>

#if defined(BSP_USING_OTG)
#include <rtdevice.h>
#include "NuMicro.h"

#define LOG_TAG    "drv.otg"
#define DBG_ENABLE
#define DBG_SECTION_NAME   LOG_TAG
#define DBG_LEVEL          LOG_LVL_DBG
#define DBG_COLOR
#include <rtdbg.h>

/* This delay must be at least 10 us */
static void _usb_init_delay(void)
{
    volatile uint32_t i = 0x1000;

    while (i--)
        __NOP();

    return;
}

/* Check current usb role */
static int usb_role(void)
{
    uint32_t status = (OTG->STATUS) & (OTG_STATUS_ASHOST_Msk | OTG_STATUS_ASPERI_Msk | OTG_STATUS_IDSTS_Msk);

    if (status == (OTG_STATUS_IDSTS_Msk | OTG_STATUS_ASPERI_Msk))
        rt_kprintf("usb frame acts as peripheral\n");

    else if (status == OTG_STATUS_ASHOST_Msk)
        rt_kprintf("usb frame acts as host\n");

    else
        rt_kprintf("usb frame is unknown state: 0x%x\n", status);

    return 0;
}
MSH_CMD_EXPORT(usb_role, check usb role);

static int otg_init(void)
{
    SYS_UnlockReg();

    /* Set OTG as ID dependent role */
    SYS->USBPHY = (SYS->USBPHY & ~SYS_USBPHY_USBROLE_Msk) | SYS_USBPHY_USBEN_Msk | SYS_USBPHY_SBO_Msk | (0x2 << SYS_USBPHY_USBROLE_Pos);

    OTG_ENABLE();
    /* Enable OTG and ID detection function */
    OTG_ENABLE_PHY();
    /* Enable ID detection function */
    OTG_ENABLE_ID_DETECT();
    /* Enable OTG interrupt */
    NVIC_EnableIRQ(USBOTG_IRQn);
    _usb_init_delay();

    /* clear interrupt and enable relative interrupts */
    OTG_ENABLE_INT(OTG_INTEN_IDCHGIEN_Msk | OTG_INTEN_HOSTIEN_Msk | OTG_INTEN_PDEVIEN_Msk |
                   OTG_INTEN_BVLDCHGIEN_Msk | OTG_INTEN_AVLDCHGIEN_Msk);

    return (int)RT_EOK;
}
INIT_DEVICE_EXPORT(otg_init);

/* OTG interrupt entry */
void USBOTG_IRQHandler(void)
{
    __IO uint32_t u32INTEN;

    /* enter interrupt */
    rt_interrupt_enter();

    u32INTEN = OTG->INTEN;

    /* usb id pin status change */
    if (u32INTEN & OTG_INTSTS_IDCHGIF_Msk)
    {
        if (OTG_GET_STATUS(OTG_STATUS_IDSTS_Msk))
            LOG_D("usb frame acts as peripheral\n");

        OTG_CLR_INT_FLAG(OTG_INTSTS_IDCHGIF_Msk);
        LOG_D("usb id change");
    }

    /* B-device session valid state change */
    if (u32INTEN & OTG_INTSTS_BVLDCHGIF_Msk)
    {
        if (OTG_GET_STATUS(OTG_STATUS_IDSTS_Msk) == 0)
            LOG_D("usb frame acts as host\n");

        OTG_CLR_INT_FLAG(OTG_INTSTS_BVLDCHGIF_Msk);
        LOG_D("usb b-device session valid state change");
    }

    /* Clear all interrupt flags */
    OTG->INTSTS = u32INTEN;

    /* leave interrupt */
    rt_interrupt_leave();

}

#endif /* defined(BSP_USING_OTG) */
