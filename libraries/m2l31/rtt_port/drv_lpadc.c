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

#if defined(BSP_USING_LPADC)

#include <rtdevice.h>
#include "NuMicro.h"

/* Private define ---------------------------------------------------------------*/
enum
{
    LPADC_START = -1,
#if defined(BSP_USING_LPADC0)
    LPADC0_IDX,
#endif
    LPADC_CNT
};

enum
{
    LPADC_CH_0,
    LPADC_CH_1,
    LPADC_CH_2,
    LPADC_CH_3,
    LPADC_CH_4,
    LPADC_CH_5,
    LPADC_CH_6,
    LPADC_CH_7,
    LPADC_CH_8,
    LPADC_CH_9,
    LPADC_CH_10,
    LPADC_CH_11,
    LPADC_CH_12,
    LPADC_CH_13,
    LPADC_CH_14,
    LPADC_CH_15,
    LPADC_CH_16,
    LPADC_CH_17,
    LPADC_CH_18,
    LPADC_CH_19,
    LPADC_CH_20,
    LPADC_CH_21,
    LPADC_CH_22,
    LPADC_CH_23,
    LPADC_CH_OPA0,
    LPADC_CH_OPA1,
    LPADC_CH_OPA2,
    LPADC_CH_AVDD_DIV4 = 28, // 28, AVDD/4
    LPADC_CH_VBG,            // 29, Band-gap voltage
    LPADC_CH_VTEMP,          // 30, Internal Temperature sensor
    LPADC_CH_VBAT_DIV4,      // 31, VBAT/4
    LPADC_CH_NUM
};

#define CONFIG_MAX_CHN_NUM  LPADC_CH_NUM

/* Private Typedef --------------------------------------------------------------*/
struct nu_lpadc
{
    struct rt_adc_device dev;
    char       *name;
    LPADC_T     *base;
    uint32_t    chn_msk;
    uint32_t    max_chn_num;
};
typedef struct nu_lpadc *nu_lpadc_t;

/* Private functions ------------------------------------------------------------*/
static rt_err_t nu_lpadc_enabled(struct rt_adc_device *device, rt_uint32_t channel, rt_bool_t enabled);
static rt_err_t nu_lpadc_convert(struct rt_adc_device *device, rt_uint32_t channel, rt_uint32_t *value);
static rt_uint8_t nu_lpadc_get_resolution(struct rt_adc_device *device);
static rt_int16_t nu_lpadc_get_vref(struct rt_adc_device *device);
static rt_err_t nu_lpadc_control(rt_device_t device, int cmd, void *args);

/* Private variables ------------------------------------------------------------*/

static struct nu_lpadc nu_lpadc_arr [] =
{
#if defined(BSP_USING_LPADC0)
    {
        .name = "lpadc0",
        .base = LPADC0,
        .chn_msk = 0,
        .max_chn_num = CONFIG_MAX_CHN_NUM,
    },
#endif
};

static const struct rt_adc_ops nu_lpadc_ops =
{
    .enabled = nu_lpadc_enabled,
    .convert = nu_lpadc_convert,
    .get_resolution = nu_lpadc_get_resolution,
    .get_vref = nu_lpadc_get_vref,
    .control = nu_lpadc_control
};
typedef struct rt_adc_ops *rt_adc_ops_t;

static rt_uint32_t s_u32BuiltInBandGapValue = 0;

static rt_uint8_t nu_lpadc_get_resolution(struct rt_adc_device *device)
{
    return 12; /* 12-bit */
}

/* nu_lpadc_enabled - Enable ADC clock and wait for ready */
static rt_err_t nu_lpadc_enabled(struct rt_adc_device *device, rt_uint32_t channel, rt_bool_t enabled)
{
    nu_lpadc_t psNuLPADC = (nu_lpadc_t)device;

    RT_ASSERT(device);

    if (channel >= psNuLPADC->max_chn_num)
        return -(RT_EINVAL);

    if (enabled)
    {
        uint32_t u32ChnMsk = psNuLPADC->chn_msk | (0x1 << channel);

        if (psNuLPADC->chn_msk != u32ChnMsk)
        {
            LPADC_Close(psNuLPADC->base);

            LPADC_POWER_ON(psNuLPADC->base);

            /* Set input mode as single-end, Single-cycle scan mode and select channels */
            LPADC_Open(psNuLPADC->base, LPADC_ADCR_DIFFEN_SINGLE_END, LPADC_ADCR_ADMD_SINGLE_CYCLE, u32ChnMsk);

            switch (channel)
            {
            case LPADC_CH_AVDD_DIV4:
                /* Enable AVDD/4 */
                SYS->IVSCTL |= SYS_IVSCTL_AVDDDIV4EN_Msk;
                break;

            case LPADC_CH_VBG:
                SYS_UnlockReg();
                /* Force to enable internal voltage band-gap. */
                SYS->VREFCTL |= SYS_VREFCTL_VBGFEN_Msk;
                break;

            case LPADC_CH_VTEMP:
                /* Enable temperature sensor */
                SYS->IVSCTL |= SYS_IVSCTL_VTEMPEN_Msk;
                break;

            case LPADC_CH_VBAT_DIV4:
                /* Enable VBAT/4 */
                SYS->IVSCTL |= SYS_IVSCTL_VBATUGEN_Msk;
                break;

            default:
                break;
            }

            psNuLPADC->chn_msk = u32ChnMsk;
        }
    }
    else
    {
        psNuLPADC->chn_msk &= ~(0x1 << channel);

        switch (channel)
        {
        case LPADC_CH_AVDD_DIV4:
            /* Disable AVDD/4 */
            SYS->IVSCTL &= ~SYS_IVSCTL_AVDDDIV4EN_Msk;
            break;

        case LPADC_CH_VBG:
            SYS_UnlockReg();
            /* Force to enable internal voltage band-gap. */
            SYS->VREFCTL &= ~SYS_VREFCTL_VBGFEN_Msk;
            break;

        case LPADC_CH_VTEMP:
            /* Disable temperature sensor */
            SYS->IVSCTL &= ~SYS_IVSCTL_VTEMPEN_Msk;
            break;

        case LPADC_CH_VBAT_DIV4:
            /* Disable VBAT/4 */
            SYS->IVSCTL &= ~SYS_IVSCTL_VBATUGEN_Msk;
            break;

        default:
            break;
        }

        if (psNuLPADC->chn_msk == 0)
        {
            LPADC_Close(psNuLPADC->base);

            LPADC_POWER_DOWN(psNuLPADC->base);
        }
    }

    return RT_EOK;
}

static rt_uint32_t _lpadc_convert(nu_lpadc_t psNuLPADC, rt_uint32_t channel)
{
#define CONFIG_EXT_SMPL_TIME           20

    if (nu_lpadc_enabled((struct rt_adc_device *)psNuLPADC, channel, RT_TRUE) != RT_EOK)
    {
        return 0xFFFFFFFF;
    }

    /* Clear the A/D interrupt flag for safe */
    LPADC_CLR_INT_FLAG(psNuLPADC->base, LPADC_ADF_INT);

    /* Set sample module extended sampling time. */
    LPADC_SetExtendSampleTime(psNuLPADC->base, 0, CONFIG_EXT_SMPL_TIME);

    /* Enable the sample module interrupt */
    LPADC_EnableInt(psNuLPADC->base, LPADC_ADF_INT);

    /* Trigger to start A/D conversion */
    LPADC_START_CONV(psNuLPADC->base);

    /* Polling the ADF flag. */
    while (LPADC_GET_INT_FLAG(psNuLPADC->base, LPADC_ADF_INT) == 0);

    /* Disable the sample module interrupt */
    LPADC_DisableInt(psNuLPADC->base, LPADC_ADF_INT);

    /* Get the conversion result. */
    return LPADC_GET_CONVERSION_DATA(psNuLPADC->base, channel);
}

static rt_int16_t nu_lpadc_get_vref(struct rt_adc_device *device)
{
    nu_lpadc_t psNuLPADC = (nu_lpadc_t)device;
    rt_uint16_t u32VBG = _lpadc_convert(psNuLPADC, LPADC_CH_VBG);

    /* Use Conversion result of Band-gap to calculating AVdd */
    /*
      u16Vref    s_u32BuiltInBandGapValue
    ---------- = -------------------------
       3072          i32ConversionData
    */
    return (3072 * s_u32BuiltInBandGapValue / u32VBG);
}

static rt_err_t nu_lpadc_convert(struct rt_adc_device *device, rt_uint32_t channel, rt_uint32_t *value)
{
    nu_lpadc_t psNuLPADC = (nu_lpadc_t)device;
    rt_err_t ret = RT_ERROR;

    RT_ASSERT(device);
    RT_ASSERT(value);

    if (channel >= psNuLPADC->max_chn_num)
    {
        *value = 0xFFFFFFFF;
        ret = RT_EINVAL;
        goto exit_nu_lpadc_convert;
    }

    if ((psNuLPADC->chn_msk & (1 << channel)) == 0)
    {
        *value = 0xFFFFFFFF;
        ret = RT_EBUSY;
        goto exit_nu_lpadc_convert;
    }

    /* Get the conversion result. */
    *value = _lpadc_convert(psNuLPADC, channel);

    ret = RT_EOK;

exit_nu_lpadc_convert:

    return -(ret);
}

static rt_err_t nu_lpadc_control(rt_device_t device, int cmd, void *args)
{
    nu_lpadc_t psNuLPADC = (nu_lpadc_t)device;

    RT_ASSERT(device);

    switch (cmd)
    {
    default:
        break;
    }

    return -RT_EINVAL;
}

int rt_hw_lpadc_init(void)
{
    int i;
    rt_err_t result;

    /* Invoke ISP function to read built-in band-gap A/D conversion result*/
    SYS_UnlockReg();
    RMC_Open();
    s_u32BuiltInBandGapValue = RMC_ReadBandGap();
    RMC_Close();

    for (i = (LPADC_START + 1); i < LPADC_CNT; i++)
    {
        result = rt_hw_adc_register(&nu_lpadc_arr[i].dev, nu_lpadc_arr[i].name, &nu_lpadc_ops, NULL);
        RT_ASSERT(result == RT_EOK);
    }

    return 0;
}
INIT_BOARD_EXPORT(rt_hw_lpadc_init);

#endif //#if defined(BSP_USING_LPADC)
