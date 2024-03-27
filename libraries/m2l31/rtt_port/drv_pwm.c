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

#if defined(BSP_USING_PWM)

#define LOG_TAG                 "drv.pwm"
#define DBG_ENABLE
#define DBG_SECTION_NAME        LOG_TAG
#define DBG_LEVEL               DBG_INFO
#define DBG_COLOR
#include <rtdbg.h>

#include <stdint.h>
#include <rtdevice.h>
#include <rthw.h>
#include "NuMicro.h"

enum
{
    PWM_START = -1,
#if defined(BSP_USING_PWM0)
    PWM0_IDX,
#endif
#if defined(BSP_USING_PWM1)
    PWM1_IDX,
#endif
    PWM_CNT
};

struct nu_pwm
{
    struct rt_device_pwm dev;
    char *name;
    PWM_T *base;
    rt_int32_t period_time;
    rt_uint32_t chn_mask;
};

typedef struct nu_pwm *nu_pwm_t;

static struct nu_pwm nu_pwm_arr [] =
{
#if defined(BSP_USING_PWM0)
    {
        .name = "pwm0",
        .base = PWM0,
    },
#endif

#if defined(BSP_USING_PWM1)
    {
        .name = "pwm1",
        .base = PWM1,
    },
#endif
}; /* pwm nu_pwm */

static rt_err_t nu_pwm_control(struct rt_device_pwm *device, int cmd, void *arg);

static struct rt_pwm_ops nu_pwm_ops =
{
    .control = nu_pwm_control
};

static rt_err_t nu_pwm_enable(struct rt_device_pwm *device, struct rt_pwm_configuration *conf, rt_bool_t enable)
{
    nu_pwm_t psNuPwm = (nu_pwm_t)device;

    if (enable == RT_TRUE)
    {
        PWM_EnableOutput(psNuPwm->base, 1 << conf->channel);
        PWM_Start(psNuPwm->base, 1 << conf->channel);

        psNuPwm->chn_mask |= (1 << conf->channel);
    }
    else
    {
        PWM_DisableOutput(psNuPwm->base, 1 << conf->channel);
        PWM_ForceStop(psNuPwm->base, 1 << conf->channel);

        psNuPwm->chn_mask &= ~(1 << conf->channel);
    }

    return RT_EOK;
}

static rt_err_t nu_pwm_set(struct rt_device_pwm *device, struct rt_pwm_configuration *conf)
{
    nu_pwm_t psNuPwm = (nu_pwm_t)device;
    rt_uint32_t pulse, pair, freq, dutycycle, period;

    if (conf->period == 0)
        return -(RT_ERROR);

    period = conf->period;
    pulse  = conf->pulse;

    if ((conf->channel % 2) == 0)
        pair = conf->channel + 1;
    else
        pair = conf->channel - 1;

    if ((psNuPwm->chn_mask & (1 << pair)) && (PWM_GET_CNR(psNuPwm->base, pair) != 0))
    {
        period = psNuPwm->period_time;
        LOG_I("%s output frequency is determined, user can only change the duty\n", psNuPwm->name);
    }
    else
    {
        psNuPwm->period_time = period;
    }

    freq = (uint64_t)1000000000 / period;
    dutycycle = (pulse * 100) / period;
    PWM_ConfigOutputChannel(psNuPwm->base, conf->channel, freq, dutycycle) ;

    LOG_I("[%s, %s] Period: %d", __func__, psNuPwm->name, conf->period);
    LOG_I("[%s, %s] Pulse: %d", __func__, psNuPwm->name, conf->pulse);

    LOG_I("[%s, %s] Freq: %d", __func__, psNuPwm->name, freq);
    LOG_I("[%s, %s] Duty: %d", __func__, psNuPwm->name, dutycycle);

    return RT_EOK;
}

static rt_uint32_t nu_pwm_clksr(struct rt_device_pwm *device)
{
    nu_pwm_t psNuPwm = (nu_pwm_t)device;
    rt_uint32_t u32Src, u32PWMClockSrc;
    PWM_T *base = psNuPwm->base;

    if (psNuPwm->base == PWM0)
    {
        u32Src = CLK->CLKSEL3 & CLK_CLKSEL3_PWM0SEL_Msk;
    }
    else//(pwm == PWM1)
    {
        u32Src = CLK->CLKSEL3 & CLK_CLKSEL3_PWM1SEL_Msk;
    }

    if (u32Src == 0)
    {
        /* clock source is from HCLK0 */
        u32PWMClockSrc = CLK_GetHCLKFreq();
    }
    else
    {
        /* clock source is from PCLK */
        SystemCoreClockUpdate();
        if (psNuPwm->base == PWM0)
        {
            u32PWMClockSrc = CLK_GetPCLK0Freq();
        }
        else     /* (pwm == PWM1) */
        {
            u32PWMClockSrc = CLK_GetPCLK1Freq();
        }
    }

    return u32PWMClockSrc;
}

static rt_err_t nu_pwm_get(struct rt_device_pwm *device, struct rt_pwm_configuration *conf)
{
    rt_uint32_t time_tick, u32PWMClockSrc;
    rt_uint32_t prescale, period, pulse;
    nu_pwm_t psNuPwm = (nu_pwm_t)device;

    RT_ASSERT(device);

    prescale = PWM_GET_PRESCALER(psNuPwm->base, conf->channel);
    period = PWM_GET_CNR(psNuPwm->base, conf->channel);
    pulse = PWM_GET_CMR(psNuPwm->base, conf->channel);

    u32PWMClockSrc = nu_pwm_clksr(device);
    time_tick = (uint64_t)1000000000000 / u32PWMClockSrc;

    LOG_I("[%s, %s] Prescale: %d", __func__, psNuPwm->name, prescale);
    LOG_I("[%s, %s] Period: %d", __func__, psNuPwm->name, period);
    LOG_I("[%s, %s] Pulse: %d", __func__, psNuPwm->name, pulse);
    LOG_I("[%s, %s] ModuleFreq: %d", __func__, psNuPwm->name, u32PWMClockSrc);
    LOG_I("[%s, %s] Tick: %d", __func__, psNuPwm->name, time_tick);

    conf->period = (((prescale + 1) * (period + 1)) * time_tick) / 1000;
    conf->pulse = (((prescale + 1) * pulse * time_tick)) / 1000;

    LOG_I("[%s, %s] Channel: %d", __func__, psNuPwm->name, conf->channel);
    LOG_I("[%s, %s] Period: %d", __func__, psNuPwm->name, conf->period);
    LOG_I("[%s, %s] Pulse: %d", __func__, psNuPwm->name, conf->pulse);

    return RT_EOK;
}

static rt_err_t nu_pwm_control(struct rt_device_pwm *device, int cmd, void *arg)
{
    struct rt_pwm_configuration *conf;

    RT_ASSERT(device);
    RT_ASSERT(arg);

    conf = (struct rt_pwm_configuration *)arg;

    if (conf->channel < PWM_CHANNEL_NUM)
    {
        switch (cmd)
        {
        case PWM_CMD_ENABLE:
            return nu_pwm_enable(device, conf, RT_TRUE);
        case PWM_CMD_DISABLE:
            return nu_pwm_enable(device, conf, RT_FALSE);
        case PWM_CMD_SET:
            return nu_pwm_set(device, conf);
        case PWM_CMD_GET:
            return nu_pwm_get(device, conf);
        default:
            break;
        }
    }

    return -(RT_EINVAL);
}

int rt_hw_pwm_init(void)
{
    rt_err_t ret;
    int i;

    for (i = (PWM_START + 1); i < PWM_CNT; i++)
    {
        ret = rt_device_pwm_register(&nu_pwm_arr[i].dev, nu_pwm_arr[i].name, &nu_pwm_ops, RT_NULL);
        RT_ASSERT(ret == RT_EOK);
    }

    return 0;
}
INIT_DEVICE_EXPORT(rt_hw_pwm_init);

#endif
