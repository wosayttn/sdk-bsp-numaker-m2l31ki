/**************************************************************************//**
*
* @copyright (C) 2019 Nuvoton Technology Corp. All rights reserved.
*
* SPDX-License-Identifier: Apache-2.0
*
* Change Logs:
* Date            Author       Notes
* 2020-1-16       Wayne        First version
*
******************************************************************************/

#include <rtdevice.h>
#include "NuMicro.h"
#include "drv_gpio.h"
#include "drv_common.h"

#if defined(BOARD_USING_NUTFT_QSPI_FLASH)
#include "spi_flash.h"
#include "spi_flash_sfud.h"
#include "drv_qspi.h"

#define W25X_REG_READSTATUS    (0x05)
#define W25X_REG_READSTATUS2   (0x35)
#define W25X_REG_WRITEENABLE   (0x06)
#define W25X_REG_WRITESTATUS   (0x01)
#define W25X_REG_QUADENABLE    (0x02)

static rt_uint8_t SpiFlash_ReadStatusReg(struct rt_qspi_device *qspi_device)
{
    rt_uint8_t u8Val;
    rt_err_t result = RT_EOK;
    rt_uint8_t w25x_txCMD1 = W25X_REG_READSTATUS;

    result = rt_qspi_send_then_recv(qspi_device, &w25x_txCMD1, 1, &u8Val, 1);
    RT_ASSERT(result > 0);

    return u8Val;
}

static rt_uint8_t SpiFlash_ReadStatusReg2(struct rt_qspi_device *qspi_device)
{
    rt_uint8_t u8Val;
    rt_err_t result = RT_EOK;
    rt_uint8_t w25x_txCMD1 = W25X_REG_READSTATUS2;

    result = rt_qspi_send_then_recv(qspi_device, &w25x_txCMD1, 1, &u8Val, 1);
    RT_ASSERT(result > 0);

    return u8Val;
}

static rt_err_t SpiFlash_WriteStatusReg(struct rt_qspi_device *qspi_device, uint8_t u8Value1, uint8_t u8Value2)
{
    rt_uint8_t w25x_txCMD1;
    rt_uint8_t au8Val[2];
    rt_err_t result;
    struct rt_qspi_message qspi_message = {0};

    /* Enable WE */
    w25x_txCMD1 = W25X_REG_WRITEENABLE;
    result = rt_qspi_send(qspi_device, &w25x_txCMD1, sizeof(w25x_txCMD1));
    if (result != sizeof(w25x_txCMD1))
        goto exit_SpiFlash_WriteStatusReg;

    /* Prepare status-1, 2 data */
    au8Val[0] = u8Value1;
    au8Val[1] = u8Value2;

    /* 1-bit mode: Instruction+payload */
    qspi_message.instruction.content = W25X_REG_WRITESTATUS;
    qspi_message.instruction.qspi_lines = 1;

    qspi_message.qspi_data_lines   = 1;
    qspi_message.parent.cs_take    = 1;
    qspi_message.parent.cs_release = 1;
    qspi_message.parent.send_buf   = &au8Val[0];
    qspi_message.parent.length     = sizeof(au8Val);
    qspi_message.parent.next       = RT_NULL;

    if (rt_qspi_transfer_message(qspi_device, &qspi_message) != sizeof(au8Val))
    {
        result = -RT_ERROR;
    }

    result  = RT_EOK;

exit_SpiFlash_WriteStatusReg:

    return result;
}

static void SpiFlash_WaitReady(struct rt_qspi_device *qspi_device)
{
    volatile uint8_t u8ReturnValue;

    do
    {
        u8ReturnValue = SpiFlash_ReadStatusReg(qspi_device);
        u8ReturnValue = u8ReturnValue & 1;
    }
    while (u8ReturnValue != 0);   // check the BUSY bit
}

static void SpiFlash_EnterQspiMode(struct rt_qspi_device *qspi_device)
{
    rt_err_t result = RT_EOK;

    uint8_t u8Status1 = SpiFlash_ReadStatusReg(qspi_device);
    uint8_t u8Status2 = SpiFlash_ReadStatusReg2(qspi_device);

    u8Status2 |= W25X_REG_QUADENABLE;

    result = SpiFlash_WriteStatusReg(qspi_device, u8Status1, u8Status2);
    RT_ASSERT(result == RT_EOK);

    SpiFlash_WaitReady(qspi_device);
}

static void SpiFlash_ExitQspiMode(struct rt_qspi_device *qspi_device)
{
    rt_err_t result = RT_EOK;
    uint8_t u8Status1 = SpiFlash_ReadStatusReg(qspi_device);
    uint8_t u8Status2 = SpiFlash_ReadStatusReg2(qspi_device);

    u8Status2 &= ~W25X_REG_QUADENABLE;

    result = SpiFlash_WriteStatusReg(qspi_device, u8Status1, u8Status2);
    RT_ASSERT(result == RT_EOK);

    SpiFlash_WaitReady(qspi_device);
}

static int rt_hw_spiflash_init(void)
{
    /* Here, we use Dual I/O to drive the SPI flash by default. */
    /* If you want to use Quad I/O, you can modify to 4 from 2 and crossover D2/D3 pin of SPI flash. */
    if (nu_qspi_bus_attach_device("qspi0", "qspi01", 2, SpiFlash_EnterQspiMode, SpiFlash_ExitQspiMode) != RT_EOK)
        return -1;

#if defined(RT_USING_SFUD)
    if (rt_sfud_flash_probe(FAL_USING_NOR_FLASH_DEV_NAME, "qspi01") == RT_NULL)
    {
        return -(RT_ERROR);
    }
#endif
    return 0;
}
INIT_COMPONENT_EXPORT(rt_hw_spiflash_init);
#endif /* BOARD_USING_NUTFT_QSPI_FLASH */


#if defined(BOARD_USING_NUTFT_ADC_TOUCH) && defined(NU_PKG_USING_ADC_TOUCH_SW)

#include "adc_touch.h"
#include "touch_sw.h"

S_CALIBRATION_MATRIX g_sCalMat = { 36, 6227, -2817292, 4936, -37, -1969986, 65536 };

static void tp_switch_to_analog(rt_base_t pin)
{
    GPIO_T *port = (GPIO_T *)(GPIOA_BASE + (0x40) * NU_GET_PORT(pin));

    if (pin == NU_GET_PININDEX(NU_PB, 7))
        nu_pin_set_function(pin, SYS_GPB_MFP1_PB7MFP_EADC0_CH7);
    else if (pin == NU_GET_PININDEX(NU_PB, 4))
        nu_pin_set_function(pin, SYS_GPB_MFP1_PB4MFP_EADC0_CH4);

    GPIO_DISABLE_DIGITAL_PATH(port, NU_GET_PIN_MASK(NU_GET_PINS(pin)));

}

static void tp_switch_to_digital(rt_base_t pin)
{
    GPIO_T *port = (GPIO_T *)(GPIOA_BASE + (0x40) * NU_GET_PORT(pin));

    nu_pin_set_function(pin, 0);

    /* Enable digital path on these EADC pins */
    GPIO_ENABLE_DIGITAL_PATH(port, NU_GET_PIN_MASK(NU_GET_PINS(pin)));
}

static S_TOUCH_SW sADCTP =
{
    .adc_name    = "eadc0",
    .i32ADCChnYU = 7,
    .i32ADCChnXR = 4,
    .pin =
    {
        NU_GET_PININDEX(NU_PB, 6), // XL
        NU_GET_PININDEX(NU_PB, 7), // YU
        NU_GET_PININDEX(NU_PB, 4), // XR
        NU_GET_PININDEX(NU_PB, 5), // YD
    },
    .switch_to_analog  = tp_switch_to_analog,
    .switch_to_digital = tp_switch_to_digital,
};

#endif /* defined(BOARD_USING_NUTFT_ADC_TOUCH) && defined(NU_PKG_USING_ADC_TOUCH_SW) */



#if defined(BOARD_USING_LCD_ILI9341) && defined(NU_PKG_USING_ILI9341_SPI)

#include <lcd_ili9341.h>
#if defined(PKG_USING_GUIENGINE)
    #include <rtgui/driver.h>
#endif

int rt_hw_ili9341_port(void)
{
    if (rt_hw_lcd_ili9341_spi_init("spi0", (void *)RT_NULL) != RT_EOK)
        return -1;

    rt_hw_lcd_ili9341_init();

#if defined(PKG_USING_GUIENGINE)
    rt_device_t lcd_ili9341;
    lcd_ili9341 = rt_device_find("lcd");
    if (lcd_ili9341)
    {
        rtgui_graphic_set_device(lcd_ili9341);
    }
#endif

#if defined(BOARD_USING_NUTFT_ADC_TOUCH) && defined(NU_PKG_USING_ADC_TOUCH_SW)
    nu_adc_touch_sw_register(&sADCTP);
#endif

    return 0;
}
INIT_COMPONENT_EXPORT(rt_hw_ili9341_port);
#endif /* defined(BOARD_USING_LCD_ILI9341) && defined(NU_PKG_USING_ILI9341_SPI) */
