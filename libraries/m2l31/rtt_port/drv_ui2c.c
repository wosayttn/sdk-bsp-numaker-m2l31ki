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

#if defined(BSP_USING_UI2C) && defined(RT_USING_I2C)

#include <rtdevice.h>
#include "NuMicro.h"

/* Private define ---------------------------------------------------------------*/
#define LOG_TAG          "drv.ui2c"
#define DBG_ENABLE
#define DBG_SECTION_NAME LOG_TAG
#define DBG_LEVEL        DBG_INFO
#define DBG_COLOR
#include <rtdbg.h>

#define SLV_10BIT_ADDR (0x1E<<2)             //1111+0xx+r/w
enum
{
    UI2C_START = -1,
#if defined(BSP_USING_UI2C0)
    UI2C0_IDX,
#endif
#if defined(BSP_USING_UI2C1)
    UI2C1_IDX,
#endif
    UI2C_CNT
};

/* Private typedef --------------------------------------------------------------*/
struct nu_ui2c_bus
{
    struct rt_i2c_bus_device parent;
    UI2C_T *UI2C;
    struct rt_i2c_msg *msg;
    char *device_name;
};
typedef struct nu_ui2c_bus *nu_ui2c_bus_t;

/* Private variables ------------------------------------------------------------*/

static struct nu_ui2c_bus nu_ui2c_arr [ ] =
{
#if defined(BSP_USING_UI2C0)
    {
        .UI2C = UI2C0, .device_name = "ui2c0",
    },
#endif
#if defined(BSP_USING_UI2C1)
    {
        .UI2C = UI2C1, .device_name = "ui2c1",
    },
#endif
};
/* Private functions ------------------------------------------------------------*/
static rt_size_t nu_ui2c_mst_xfer(struct rt_i2c_bus_device *bus,
                                  struct rt_i2c_msg msgs[],
                                  rt_uint32_t num);

static rt_err_t nu_ui2c_bus_control(struct rt_i2c_bus_device *bus,
                                    rt_uint32_t u32Cmd,
                                    rt_uint32_t u32Value);

static const struct rt_i2c_bus_device_ops nu_ui2c_ops =
{
    .master_xfer        = nu_ui2c_mst_xfer,
    .slave_xfer         = NULL,
    .i2c_bus_control    = nu_ui2c_bus_control,
};

static rt_err_t nu_ui2c_bus_control(struct rt_i2c_bus_device *bus, rt_uint32_t u32Cmd, rt_uint32_t u32Value)
{
    nu_ui2c_bus_t nu_ui2c;

    RT_ASSERT(bus);
    nu_ui2c = (nu_ui2c_bus_t) bus;

    switch (u32Cmd)
    {
    case RT_I2C_DEV_CTRL_CLK:
        UI2C_SetBusClockFreq(nu_ui2c->UI2C, u32Value);
        break;
    default:
        return -RT_EIO;
    }

    return RT_EOK;
}

static inline rt_err_t nu_ui2c_wait_ready_with_timeout(nu_ui2c_bus_t nu_ui2c)
{
    rt_tick_t start = rt_tick_get();
    while (!(UI2C_GET_PROT_STATUS(nu_ui2c->UI2C) & (UI2C_PROTSTS_STARIF_Msk | UI2C_PROTSTS_ACKIF_Msk | UI2C_PROTSTS_NACKIF_Msk | UI2C_PROTSTS_STORIF_Msk)))
    {
        if ((rt_tick_get() - start) > nu_ui2c->parent.timeout)
        {
            LOG_E("\nui2c: timeout!\n");
            return -RT_ETIMEOUT;
        }
    }

    return RT_EOK;
}

static inline rt_err_t nu_ui2c_send_data(nu_ui2c_bus_t nu_ui2c, rt_uint8_t data)
{
    UI2C_SET_DATA(nu_ui2c->UI2C, data);
    UI2C_SET_CONTROL_REG(nu_ui2c->UI2C, UI2C_CTL_PTRG);
    return nu_ui2c_wait_ready_with_timeout(nu_ui2c);
}

static rt_err_t nu_ui2c_send_address(nu_ui2c_bus_t nu_ui2c,
                                     struct rt_i2c_msg  *msg)
{
    rt_uint16_t flags = msg->flags;
    rt_uint16_t ignore_nack = msg->flags & RT_I2C_IGNORE_NACK;
    rt_uint8_t addr1, addr2;
    rt_err_t ret;

    if (flags & RT_I2C_ADDR_10BIT)
    {
        UI2C_ENABLE_10BIT_ADDR_MODE(nu_ui2c->UI2C);
        /* Init Send 10-bit Addr */
        addr1 = ((msg->addr >> 8) | SLV_10BIT_ADDR) << 1;
        addr2 = msg->addr & 0xff;

        LOG_D("addr1: %d, addr2: %d\n", addr1, addr2);

        ret = nu_ui2c_send_data(nu_ui2c, addr1);
        if (ret != RT_EOK) //for timeout condition
            return -RT_EIO;

        if (((UI2C_GET_PROT_STATUS(nu_ui2c->UI2C) & UI2C_PROTSTS_ACKIF_Msk) != UI2C_PROTSTS_ACKIF_Msk) && !ignore_nack)
        {
            LOG_E("NACK: sending first addr\n");

            return -RT_EIO;
        }
        UI2C_CLR_PROT_INT_FLAG(nu_ui2c->UI2C, UI2C_PROTSTS_ACKIF_Msk);

        ret = nu_ui2c_send_data(nu_ui2c,  addr2);
        if (ret != RT_EOK) //for timeout condition
            return -RT_EIO;

        if (((UI2C_GET_PROT_STATUS(nu_ui2c->UI2C) & UI2C_PROTSTS_ACKIF_Msk) != UI2C_PROTSTS_ACKIF_Msk) && !ignore_nack)
        {
            LOG_E("NACK: sending second addr\n");

            return -RT_EIO;
        }
        UI2C_CLR_PROT_INT_FLAG(nu_ui2c->UI2C, UI2C_PROTSTS_ACKIF_Msk);

        if (flags & RT_I2C_RD)
        {
            LOG_D("send repeated start condition\n");

            UI2C_SET_CONTROL_REG(nu_ui2c->UI2C, (UI2C_CTL_PTRG | UI2C_CTL_STA));
            ret = nu_ui2c_wait_ready_with_timeout(nu_ui2c);
            if (ret != RT_EOK) //for timeout condition
                return -RT_EIO;

            if (((UI2C_GET_PROT_STATUS(nu_ui2c->UI2C) & UI2C_PROTSTS_STARIF_Msk) != UI2C_PROTSTS_STARIF_Msk) && !ignore_nack)
            {
                LOG_E("sending repeated START fail\n");

                return -RT_EIO;
            }
            UI2C_CLR_PROT_INT_FLAG(nu_ui2c->UI2C, UI2C_PROTSTS_STARIF_Msk);

            addr1 |= RT_I2C_RD;

            ret = nu_ui2c_send_data(nu_ui2c,  addr1);
            if (ret != RT_EOK) //for timeout condition
                return -RT_EIO;

            if (((UI2C_GET_PROT_STATUS(nu_ui2c->UI2C) & UI2C_PROTSTS_ACKIF_Msk) != UI2C_PROTSTS_ACKIF_Msk) && !ignore_nack)
            {
                LOG_E("NACK: sending repeated addr\n");
                return -RT_EIO;
            }
            UI2C_CLR_PROT_INT_FLAG(nu_ui2c->UI2C, UI2C_PROTSTS_ACKIF_Msk);
        }
    }
    else
    {
        /* 7-bit addr */
        addr1 = msg->addr << 1;
        if (flags & RT_I2C_RD)
            addr1 |= RT_I2C_RD;

        /* Send device address */
        ret = nu_ui2c_send_data(nu_ui2c,  addr1); /* Send Address */
        if (ret != RT_EOK) //for timeout condition
            return -RT_EIO;

        if (((UI2C_GET_PROT_STATUS(nu_ui2c->UI2C) & UI2C_PROTSTS_ACKIF_Msk) != UI2C_PROTSTS_ACKIF_Msk)
                && !ignore_nack)
        {
            LOG_E("sending addr fail\n");
            return -RT_EIO;
        }
        UI2C_CLR_PROT_INT_FLAG(nu_ui2c->UI2C, UI2C_PROTSTS_ACKIF_Msk);
    }

    return RT_EOK;
}

static rt_size_t nu_ui2c_mst_xfer(struct rt_i2c_bus_device *bus,
                                  struct rt_i2c_msg msgs[],
                                  rt_uint32_t num)
{
    struct rt_i2c_msg *msg;
    nu_ui2c_bus_t nu_ui2c;
    rt_size_t i;
    rt_uint32_t cnt_data;
    rt_uint16_t ignore_nack;
    rt_err_t ret;

    RT_ASSERT(bus != RT_NULL);
    nu_ui2c = (nu_ui2c_bus_t) bus;

    nu_ui2c->msg = msgs;

    (nu_ui2c->UI2C)->PROTSTS = (nu_ui2c->UI2C)->PROTSTS;//Clear status

    UI2C_SET_CONTROL_REG(nu_ui2c->UI2C, UI2C_CTL_STA);
    ret = nu_ui2c_wait_ready_with_timeout(nu_ui2c);

    if (ret != RT_EOK) //for timeout condition
    {
        rt_set_errno(-RT_ETIMEOUT);
        return 0;
    }

    if (((UI2C_GET_PROT_STATUS(nu_ui2c->UI2C) & UI2C_PROTSTS_STARIF_Msk) != UI2C_PROTSTS_STARIF_Msk)) /* Check Send START */
    {
        i = 0;
        LOG_E("Send START Fail");
        return i;
    }
    UI2C_CLR_PROT_INT_FLAG(nu_ui2c->UI2C, UI2C_PROTSTS_STARIF_Msk);

    for (i = 0; i < num; i++)
    {
        msg = &msgs[i];
        ignore_nack = msg->flags & RT_I2C_IGNORE_NACK;

        if (!(msg->flags & RT_I2C_NO_START))
        {
            if (i)
            {
                UI2C_SET_CONTROL_REG(nu_ui2c->UI2C, (UI2C_CTL_PTRG | UI2C_CTL_STA));/* Send repeat START */
                ret = nu_ui2c_wait_ready_with_timeout(nu_ui2c);
                if (ret != RT_EOK) //for timeout condition
                    break;

                if (((UI2C_GET_PROT_STATUS(nu_ui2c->UI2C) & UI2C_PROTSTS_STARIF_Msk) != UI2C_PROTSTS_STARIF_Msk)) /* Check Send repeat START */
                {
                    i = 0;
                    LOG_E("Send repeat START Fail");
                    break;
                }
                UI2C_CLR_PROT_INT_FLAG(nu_ui2c->UI2C, UI2C_PROTSTS_STARIF_Msk);
            }

            if ((RT_EOK != nu_ui2c_send_address(nu_ui2c, msg))
                    && !ignore_nack)
            {
                i = 0;
                LOG_E("Send Address Fail");
                break;
            }
        }

        if (nu_ui2c->msg[i].flags & RT_I2C_RD) /* Receive Bytes */
        {
            rt_uint32_t do_rd_nack = (i == (num - 1));
            for (cnt_data = 0 ; cnt_data < (nu_ui2c->msg[i].len) ; cnt_data++)
            {
                do_rd_nack += (cnt_data == (nu_ui2c->msg[i].len - 1)); /* NACK after last byte  for hardware setting */
                if (do_rd_nack == 2)
                {
                    UI2C_SET_CONTROL_REG(nu_ui2c->UI2C, UI2C_CTL_PTRG);
                }
                else
                {
                    UI2C_SET_CONTROL_REG(nu_ui2c->UI2C, (UI2C_CTL_PTRG | UI2C_CTL_AA));
                }

                ret = nu_ui2c_wait_ready_with_timeout(nu_ui2c);
                if (ret != RT_EOK) //for timeout condition
                    break;

                if (nu_ui2c->UI2C->PROTCTL & UI2C_CTL_AA)
                {
                    if (((UI2C_GET_PROT_STATUS(nu_ui2c->UI2C) & UI2C_PROTSTS_ACKIF_Msk) != UI2C_PROTSTS_ACKIF_Msk)) /*Master Receive Data ACK*/
                    {
                        i = 0;
                        break;
                    }
                    UI2C_CLR_PROT_INT_FLAG(nu_ui2c->UI2C, UI2C_PROTSTS_ACKIF_Msk);
                }
                else
                {
                    if (((UI2C_GET_PROT_STATUS(nu_ui2c->UI2C) & UI2C_PROTSTS_NACKIF_Msk) != UI2C_PROTSTS_NACKIF_Msk)) /*Master Receive Data NACK*/
                    {
                        i = 0;
                        break;
                    }
                    UI2C_CLR_PROT_INT_FLAG(nu_ui2c->UI2C, UI2C_PROTSTS_NACKIF_Msk);
                }

                nu_ui2c->msg[i].buf[cnt_data] = nu_ui2c->UI2C->RXDAT;
            }
        }
        else /* Send Bytes */
        {
            for (cnt_data = 0 ; cnt_data < (nu_ui2c->msg[i].len) ; cnt_data++)
            {
                /* Send register number and MSB of data */
                ret = nu_ui2c_send_data(nu_ui2c, (uint8_t)(nu_ui2c->msg[i].buf[cnt_data]));
                if (ret != RT_EOK) //for timeout condition
                    break;

                if (((UI2C_GET_PROT_STATUS(nu_ui2c->UI2C) & UI2C_PROTSTS_ACKIF_Msk) != UI2C_PROTSTS_ACKIF_Msk)
                        && !ignore_nack
                   ) /* Send data and get Ack */
                {
                    i = 0;
                    break;
                }
                UI2C_CLR_PROT_INT_FLAG(nu_ui2c->UI2C, UI2C_PROTSTS_ACKIF_Msk);
            }
        }
    }

    UI2C_SET_CONTROL_REG(nu_ui2c->UI2C, (UI2C_CTL_PTRG | UI2C_CTL_STO));            /* Send STOP signal */
    ret = nu_ui2c_wait_ready_with_timeout(nu_ui2c);
    if (ret != RT_EOK) //for timeout condition
    {
        rt_set_errno(-RT_ETIMEOUT);
        return 0;
    }

    RT_ASSERT(((UI2C_GET_PROT_STATUS(nu_ui2c->UI2C) & UI2C_PROTSTS_STORIF_Msk) == UI2C_PROTSTS_STORIF_Msk));
    if (((UI2C_GET_PROT_STATUS(nu_ui2c->UI2C) & UI2C_PROTSTS_STORIF_Msk) != UI2C_PROTSTS_STORIF_Msk)) /* Bus Free*/
    {
        i = 0;
        LOG_E("Send STOP Fail");
    }

    UI2C_CLR_PROT_INT_FLAG(nu_ui2c->UI2C, UI2C_PROTSTS_STORIF_Msk);
    UI2C_SET_CONTROL_REG(nu_ui2c->UI2C, UI2C_CTL_PTRG);
    UI2C_DISABLE_10BIT_ADDR_MODE(nu_ui2c->UI2C); /*clear all sub modes like 10 bit mode*/
    nu_ui2c->msg = RT_NULL;

    return i;
}
/* Public functions -------------------------------------------------------------*/
int rt_hw_ui2c_init(void)
{
    int i;
    rt_err_t ret = RT_EOK;

    for (i = (UI2C_START + 1); i < UI2C_CNT; i++)
    {
        /* Reset and initial IP engine. */
        UI2C_Close(nu_ui2c_arr[i].UI2C);
        UI2C_Open(nu_ui2c_arr[i].UI2C, 100000);

        nu_ui2c_arr[i].parent.ops = &nu_ui2c_ops;

        ret = rt_i2c_bus_device_register(&nu_ui2c_arr[i].parent, nu_ui2c_arr[i].device_name);
        RT_ASSERT(RT_EOK == ret);
    }

    return 0;
}

INIT_DEVICE_EXPORT(rt_hw_ui2c_init);

#endif //#if defined(BSP_USING_UI2C)
