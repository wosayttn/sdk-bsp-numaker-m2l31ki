/**************************************************************************//**
*
* @copyright (C) 2019 Nuvoton Technology Corp. All rights reserved.
*
* SPDX-License-Identifier: Apache-2.0
*
* Change Logs:
* Date            Author       Notes
* 2024-3-11       Wayne        First version
*
******************************************************************************/

#include <rtthread.h>
#include <rtdevice.h>
#include <sys/time.h>
#include "drv_gpio.h"
#include "NuMicro.h"

#if defined(RT_USING_PIN)

/* defined the BTN0 pin: PB2 */
#define BTN0   NU_GET_PININDEX(NU_PB, 2)

/* defined the BTN1 pin: PB3 */
#define BTN1   NU_GET_PININDEX(NU_PB, 3)

static void nu_btn0_event_cb(void *args)
{
    uint32_t u32PinIdxValue = (uint32_t)args;
    static int32_t i32Count = 0;

    rt_kprintf("%d pressed!!(%d)\n", u32PinIdxValue, i32Count);

#if defined(RT_USING_PM)
    if (i32Count++ > 4)
    {
        rt_kprintf("Back to normal\n");

        /* Request normal mode */
        rt_pm_request(PM_SLEEP_MODE_NONE);

        i32Count = 0;
    }
#endif
}

#if defined(RT_USING_PM)
static int pm_cko_init(void)
{
    rt_kprintf("Enable CLK0 function to observe HCLK/4 waveform.\n");

    CLK_EnableCKO(CLK_CLKSEL1_CLKOSEL_HCLK, 3, 0);
    SYS->GPC_MFP3 = (SYS->GPC_MFP3 & ~SYS_GPC_MFP3_PC13MFP_Msk) | SYS_GPC_MFP3_PC13MFP_CLKO;

    return 0;
}
INIT_APP_EXPORT(pm_cko_init);
#endif

static int pm_test_pin(void)
{
    /* Configure BTN0/BTN1 as Input mode and enable interrupt by rising edge trigger */
    rt_pin_mode(BTN0, PIN_MODE_INPUT);
    rt_pin_attach_irq(BTN0, PIN_IRQ_MODE_FALLING, nu_btn0_event_cb, (void *)BTN0);
    rt_pin_irq_enable(BTN0, PIN_IRQ_ENABLE);

    rt_pin_mode(BTN1, PIN_MODE_INPUT);
    rt_pin_attach_irq(BTN1, PIN_IRQ_MODE_FALLING, nu_btn0_event_cb, (void *)BTN1);
    rt_pin_irq_enable(BTN1, PIN_IRQ_ENABLE);

    /* Enable interrupt de-bounce function and select de-bounce sampling cycle time is 1024 clocks of LIRC clock */
    GPIO_SET_DEBOUNCE_TIME(PB, GPIO_DBCTL_DBCLKSRC_LIRC, GPIO_DBCTL_DBCLKSEL_1024);
    GPIO_ENABLE_DEBOUNCE(PB, BIT2 | BIT3);

    return 0;
}
INIT_APP_EXPORT(pm_test_pin);
#endif

#if defined(RT_USING_ALARM)

#define ENCODE_TM(tm, year, mon, mday, hour, min, sec)      \
{                                                           \
    (tm).tm_year = (year) - 1900;                           \
    (tm).tm_mon  = (mon) - 1;                               \
    (tm).tm_mday = (mday);                                  \
    (tm).tm_hour = (hour);                                  \
    (tm).tm_min  = (min);                                   \
    (tm).tm_sec  = (sec);                                   \
}

static volatile uint32_t bWaitAlarmNotify = 0;
static void rt_alarm_cb(rt_alarm_t alarm, time_t timestamp)
{
#if defined(RT_USING_PM)
    /* Request normal mode */
    rt_pm_request(PM_SLEEP_MODE_NONE);
#endif

    bWaitAlarmNotify = 1;

    rt_kprintf("[%s/%d] alarm=0x%08x, %08x\n", __func__, rt_tick_get(), alarm, timestamp);
}

static int pm_test_rtc_alarm(void)
{
    struct rt_rtc_wkalarm wkalarm;          /* Define RTC wake-up alarm structure */
    struct rt_alarm_setup alarm_setup;      /* Define alarm setup structure */
    struct rt_alarm *alarm;                 /* Define alarm pointer */
    rt_device_t rtc_dev;                    /* Define RTC device pointer */

    struct tm tm;                           /* Define time structure */
    time_t tw = 0;                          /* Define time variable */

    /* Find the RTC device */
    if ((rtc_dev = rt_device_find("rtc")) == RT_NULL)
    {
        rt_kprintf("Can't find rtc device!\n");  /* Print error message if RTC device is not found */
        goto exit_pm_test_rtc_alarm;             /* Jump to error handling section */
    }

    /* Clear the alarm_setup.wktime structure and set it to current time */
    rt_memset(&alarm_setup.wktime, RT_ALARM_TM_NOW, sizeof(struct tm));

    /* Set the current date and time (2024-05-28 15:15:00) */
    ENCODE_TM(tm, 2024, 5, 28, 15, 15, 0);

    /* Set the wake-up date and time (2024-05-28 15:15:10) */
    ENCODE_TM(alarm_setup.wktime, 2024, 5, 28, 15, 15, 10);

    /* Convert the time to time_t format */
    tw = timegm(&tm);

    /* Set the RTC device time */
    rt_device_control(rtc_dev, RT_DEVICE_CTRL_RTC_SET_TIME, &tw);

    /* Set the alarm as one-shot */
    alarm_setup.flag = RT_ALARM_ONESHOT;

    /* Create the alarm */
    alarm = rt_alarm_create(rt_alarm_cb, &alarm_setup);

    /* Initialize the alarm notification flag */
    bWaitAlarmNotify = 0;

    /* Start the alarm and check if it is successful */
    if (alarm && (rt_alarm_start(alarm) == RT_EOK))
    {
        rt_kprintf("Sleep 10 seconds for waiting alarm occurred.\n");

#if defined(RT_USING_PM)
        rt_uint8_t mode = PM_SLEEP_MODE_STANDBY; /* Define power management mode as standby mode */
        //rt_uint8_t mode = PM_SLEEP_MODE_DEEP;  /* Define power management mode as deep sleep mode */
        rt_pm_request(mode);                     /* Request to enter the specified sleep mode */

        /* Release all power management resources */
        for (int i = (mode - 1); i >= 0; i--)
            rt_pm_release_all(i);

        /* Enter idle task to perform pm_sleep */
        rt_thread_mdelay(10);

        /* Now, the system enters deep mode and wakes up from RTC alarm */
#endif

        /* Wait for notification from the alarm callback */
        while (!bWaitAlarmNotify);

        /* Stop and delete the alarm */
        rt_alarm_stop(alarm);
        rt_alarm_delete(alarm);
    }

    return 0;

exit_pm_test_rtc_alarm:
    return -1;  /* Error handling section, return -1 to indicate failure */
}

MSH_CMD_EXPORT(pm_test_rtc_alarm, test rtc alarm);
#endif


