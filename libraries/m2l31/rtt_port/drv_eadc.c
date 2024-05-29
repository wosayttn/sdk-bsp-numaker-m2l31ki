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

#if defined(BSP_USING_EADC)

#include <rtdevice.h>
#include "NuMicro.h"

#define CONFIG_MAX_CHN_NUM  24

/* Private define ---------------------------------------------------------------*/
enum
{
    EADC_START = -1,
#if defined(BSP_USING_EADC0)
    EADC0_IDX,
#endif
    EADC_CNT
};

/* Private Typedef --------------------------------------------------------------*/
struct nu_eadc
{
    struct rt_adc_device dev;
    char       *name;
    EADC_T     *base;
    uint32_t    conv_power;
    uint32_t    chn_msk;
    uint32_t    max_chn_num;
};
typedef struct nu_eadc *nu_eadc_t;

/* Private functions ------------------------------------------------------------*/
static rt_err_t nu_eadc_enabled(struct rt_adc_device *device, rt_uint32_t channel, rt_bool_t enabled);
static rt_err_t nu_eadc_convert(struct rt_adc_device *device, rt_uint32_t channel, rt_uint32_t *value);
static rt_uint8_t nu_eadc_get_resolution(struct rt_adc_device *device);
static rt_int16_t nu_eadc_get_vref(struct rt_adc_device *device);
static rt_err_t nu_eadc_control(rt_device_t device, int cmd, void *args);

/* Private variables ------------------------------------------------------------*/

static struct nu_eadc nu_eadc_arr [] =
{
#if defined(BSP_USING_EADC0)
    {
        .name = "eadc0",
        .base = EADC0,
        .conv_power = 5,
        .chn_msk = 0,
        .max_chn_num = CONFIG_MAX_CHN_NUM,
    },
#endif
};

static const struct rt_adc_ops nu_adc_ops =
{
    .enabled = nu_eadc_enabled,
    .convert = nu_eadc_convert,
    .get_resolution = nu_eadc_get_resolution,
    .get_vref = nu_eadc_get_vref,
    .control = nu_eadc_control
};
typedef struct rt_adc_ops *rt_adc_ops_t;

static rt_uint8_t nu_eadc_get_resolution(struct rt_adc_device *device)
{
    return 12; /* 12-bit */
}

static rt_int16_t nu_eadc_get_vref(struct rt_adc_device *device)
{
    rt_uint32_t u32VRefMsk = SYS->VREFCTL & SYS_VREFCTL_VREFCTL_Msk;
    rt_uint16_t u16Vref;

    switch (u32VRefMsk)
    {
    case SYS_VREFCTL_VREF_1_6V:
        u16Vref = 1600;
        break;

    case SYS_VREFCTL_VREF_2_0V:
        u16Vref = 2000;
        break;

    case SYS_VREFCTL_VREF_2_5V:
        u16Vref = 2500;
        break;

    case SYS_VREFCTL_VREF_PIN:
    /* FALLTHROUGH */
    case SYS_VREFCTL_VREF_3_0V:
    /* FALLTHROUGH */
    default:
        u16Vref = 3000;
        break;
    }

    return u16Vref;
}

/* nu_adc_enabled - Enable ADC clock and wait for ready */
static rt_err_t nu_eadc_enabled(struct rt_adc_device *device, rt_uint32_t channel, rt_bool_t enabled)
{
    nu_eadc_t psNuEADC = (nu_eadc_t)device;

    RT_ASSERT(device);

    if (channel >= psNuEADC->max_chn_num)
        return -(RT_EINVAL);

    if (enabled)
    {
        if (psNuEADC->chn_msk == 0)
        {
            EADC_Open(psNuEADC->base, EADC_CTL_DIFFEN_SINGLE_END);
        }

        psNuEADC->chn_msk |= (0x1 << channel);
    }
    else
    {
        psNuEADC->chn_msk &= ~(0x1 << channel);

        if (psNuEADC->chn_msk == 0)
        {
            EADC_Close(psNuEADC->base);
        }
    }

    return RT_EOK;
}

static rt_err_t nu_eadc_convert(struct rt_adc_device *device, rt_uint32_t channel, rt_uint32_t *value)
{
#define CONFIG_SMPL_MODULE_IDX         0
#define CONFIG_SMPL_MODULE_ACU_TIMES   (psNuEADC->conv_power << EADC_MCTL1_ACU_Pos)

    nu_eadc_t psNuEADC = (nu_eadc_t)device;
    rt_err_t ret = RT_ERROR;

    RT_ASSERT(device);
    RT_ASSERT(value);

    if (channel >= psNuEADC->max_chn_num)
    {
        *value = 0xFFFFFFFF;
        ret = RT_EINVAL;
        goto exit_nu_eadc_convert;
    }

    if ((psNuEADC->chn_msk & (1 << channel)) == 0)
    {
        *value = 0xFFFFFFFF;
        ret = RT_EBUSY;
        goto exit_nu_eadc_convert;
    }

    /* Configure the sample module for analog input channel and software trigger source. */
    EADC_ConfigSampleModule(psNuEADC->base, CONFIG_SMPL_MODULE_IDX, EADC_SOFTWARE_TRIGGER, channel);

    /* Set sample module external sampling time to 0 */
    EADC_SetExtendSampleTime(psNuEADC->base, CONFIG_SMPL_MODULE_IDX, 0);

    /* Enable Accumulate feature */
    EADC_ENABLE_ACU(psNuEADC->base, CONFIG_SMPL_MODULE_IDX, CONFIG_SMPL_MODULE_ACU_TIMES);

    /* Enable Average feature */
    EADC_ENABLE_AVG(psNuEADC->base, CONFIG_SMPL_MODULE_IDX);

    /* Clear the A/D ADINT0 interrupt flag for safe */
    EADC_CLR_INT_FLAG(psNuEADC->base, EADC_STATUS2_ADIF0_Msk);

    /* Enable the sample module interrupt. */
    EADC_ENABLE_INT(psNuEADC->base, BIT0);
    EADC_ENABLE_SAMPLE_MODULE_INT(psNuEADC->base, CONFIG_SMPL_MODULE_IDX, BIT0);

    EADC_START_CONV(psNuEADC->base, BIT0);
    while (EADC_GET_INT_FLAG(psNuEADC->base, BIT0) == 0);

    /* Disable the sample module interrupt. */
    EADC_DISABLE_INT(psNuEADC->base, BIT0);

    /* Disable Average feature */
    EADC_DISABLE_AVG(psNuEADC->base, CONFIG_SMPL_MODULE_IDX);

    /* Disable Accumulate feature */
    EADC_DISABLE_ACU(psNuEADC->base, CONFIG_SMPL_MODULE_IDX);

    *value = EADC_GET_CONV_DATA(psNuEADC->base, CONFIG_SMPL_MODULE_IDX);

    ret = RT_EOK;

exit_nu_eadc_convert:

    return -(ret);
}

static rt_err_t nu_eadc_control(rt_device_t device, int cmd, void *args)
{
    nu_eadc_t psNuEADC = (nu_eadc_t)device;

    RT_ASSERT(device);

    switch (cmd)
    {
    case NU_ADC_CMD_SET_CONV_POWER:
    {
        uint8_t u8ConvPwr = *((uint8_t *)args);

        if ((u8ConvPwr >= 0) && (u8ConvPwr <= 8))
        {
            psNuEADC->conv_power = u8ConvPwr;
        }

        rt_kprintf("%s %d\n", __func__, psNuEADC->conv_power);

        return (psNuEADC->conv_power != u8ConvPwr) ? -RT_EINVAL : RT_EOK;
    }
    default:
        break;
    }

    return -RT_EINVAL;
}

int rt_hw_eadc_init(void)
{
    int i;
    rt_err_t result;

    for (i = (EADC_START + 1); i < EADC_CNT; i++)
    {
        result = rt_hw_adc_register(&nu_eadc_arr[i].dev, nu_eadc_arr[i].name, &nu_adc_ops, NULL);
        RT_ASSERT(result == RT_EOK);
    }

    return 0;
}
INIT_BOARD_EXPORT(rt_hw_eadc_init);

#endif //#if defined(BSP_USING_EADC)
