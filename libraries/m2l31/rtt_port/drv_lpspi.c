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

#if defined(BSP_USING_LPSPI)

#define LOG_TAG                 "drv.lpspi"
#define DBG_ENABLE
#define DBG_SECTION_NAME        LOG_TAG
#define DBG_LEVEL               DBG_INFO
#define DBG_COLOR
#include <rtdbg.h>

#include <rthw.h>
#include <rtdevice.h>

#include "drv_spi.h"

/* Private define ---------------------------------------------------------------*/

enum
{
    LPSPI_START = -1,
#if defined(BSP_USING_LPSPI0)
    LPSPI0_IDX,
#endif
    LPSPI_CNT
};

/* Private typedef --------------------------------------------------------------*/

/* Private functions ------------------------------------------------------------*/
static void nu_spi_transmission_with_poll(struct nu_spi *spi_bus,
        uint8_t *send_addr, uint8_t *recv_addr, int length, uint8_t bytes_per_word);
static int nu_spi_register_bus(struct nu_spi *spi_bus, const char *name);
static rt_uint32_t nu_spi_bus_xfer(struct rt_spi_device *device, struct rt_spi_message *message);
static rt_err_t nu_spi_bus_configure(struct rt_spi_device *device, struct rt_spi_configuration *configuration);

#if defined(BSP_USING_SPI_PDMA)
    static void nu_pdma_spi_rx_cb_event(void *pvUserData, uint32_t u32EventFilter);
    static rt_err_t nu_pdma_spi_rx_config(struct nu_spi *spi_bus, uint8_t *pu8Buf, int32_t i32RcvLen, uint8_t bytes_per_word);
    static rt_err_t nu_pdma_spi_tx_config(struct nu_spi *spi_bus, const uint8_t *pu8Buf, int32_t i32SndLen, uint8_t bytes_per_word);
    static rt_size_t nu_spi_pdma_transmit(struct nu_spi *spi_bus, const uint8_t *send_addr, uint8_t *recv_addr, int length, uint8_t bytes_per_word);
#endif
/* Public functions -------------------------------------------------------------*/

/* Private variables ------------------------------------------------------------*/
static struct rt_spi_ops nu_spi_poll_ops =
{
    .configure = nu_spi_bus_configure,
    .xfer      = nu_spi_bus_xfer,
};

static struct nu_spi nu_spi_arr [] =
{
#if defined(BSP_USING_LPSPI0)
    {
        .name = "lpspi0",
        .spi_base = (SPI_T *)LPSPI0,
        .rstidx = LPSPI0_RST,
#if defined(BSP_USING_SPI_PDMA)
#if defined(BSP_USING_LPSPI0_PDMA)
        .pdma_perp_tx = NU_PDMA_SET_REQ_SRC_ID(LPPDMA_IDX, LPPDMA_LPSPI0_TX),
        .pdma_perp_rx = NU_PDMA_SET_REQ_SRC_ID(LPPDMA_IDX, LPPDMA_LPSPI0_RX),
#else
        .pdma_perp_tx = NU_PDMA_UNUSED,
        .pdma_perp_rx = NU_PDMA_UNUSED,
#endif
#endif
    },
#endif

}; /* spi nu_spi */

static rt_err_t nu_spi_bus_configure(struct rt_spi_device *device,
                                     struct rt_spi_configuration *configuration)
{
    struct nu_spi *spi_bus;
    uint32_t u32SPIMode;
    uint32_t u32BusClock;
    rt_err_t ret = RT_EOK;
    void *pvUserData;

    RT_ASSERT(device);
    RT_ASSERT(configuration);

    spi_bus = (struct nu_spi *) device->bus;
    pvUserData = device->parent.user_data;

    /* Check mode */
    switch (configuration->mode & RT_SPI_MODE_3)
    {
    case RT_SPI_MODE_0:
        u32SPIMode = LPSPI_MODE_0;
        break;
    case RT_SPI_MODE_1:
        u32SPIMode = LPSPI_MODE_1;
        break;
    case RT_SPI_MODE_2:
        u32SPIMode = LPSPI_MODE_2;
        break;
    case RT_SPI_MODE_3:
        u32SPIMode = LPSPI_MODE_3;
        break;
    default:
        ret = RT_EIO;
        goto exit_nu_spi_bus_configure;
    }

    /* Check data width */
    if (!(configuration->data_width == 8  ||
            configuration->data_width == 16 ||
            configuration->data_width == 24 ||
            configuration->data_width == 32))
    {
        ret = RT_EINVAL;
        goto exit_nu_spi_bus_configure;
    }

    /* Try to set clock and get actual spi bus clock */
    u32BusClock = LPSPI_SetBusClock((LPSPI_T *)spi_bus->spi_base, configuration->max_hz);
    if (configuration->max_hz > u32BusClock)
    {
        LOG_W("%s clock max frequency is %dHz (!= %dHz)\n", spi_bus->name, u32BusClock, configuration->max_hz);
        configuration->max_hz = u32BusClock;
    }

    /* Need to initialize new configuration? */
    if (rt_memcmp(configuration, &spi_bus->configuration, sizeof(*configuration)) != 0)
    {
        rt_memcpy(&spi_bus->configuration, configuration, sizeof(*configuration));

        LPSPI_Open((LPSPI_T *)spi_bus->spi_base, LPSPI_MASTER, u32SPIMode, configuration->data_width, u32BusClock);

        if (configuration->mode & RT_SPI_CS_HIGH)
        {
            /* Set CS pin to LOW */
            if (pvUserData != RT_NULL)
            {
                // set to LOW */
                rt_pin_write(*((rt_base_t *)pvUserData), PIN_LOW);
            }
            else
            {
                LPSPI_SET_SS_LOW(spi_bus->spi_base);
            }
        }
        else
        {
            /* Set CS pin to HIGH */
            if (pvUserData != RT_NULL)
            {
                // set to HIGH */
                rt_pin_write(*((rt_base_t *)pvUserData), PIN_HIGH);
            }
            else
            {
                /* Set CS pin to HIGH */
                LPSPI_SET_SS_HIGH(spi_bus->spi_base);
            }
        }

        if (configuration->mode & RT_SPI_MSB)
        {
            /* Set sequence to MSB first */
            LPSPI_SET_MSB_FIRST(spi_bus->spi_base);
        }
        else
        {
            /* Set sequence to LSB first */
            LPSPI_SET_LSB_FIRST(spi_bus->spi_base);
        }
    }

    /* Clear SPI RX FIFO */
    nu_spi_drain_rxfifo(spi_bus->spi_base);

exit_nu_spi_bus_configure:

    return -(ret);
}

#if defined(BSP_USING_SPI_PDMA)
static void nu_pdma_spi_rx_cb_event(void *pvUserData, uint32_t u32EventFilter)
{
    rt_err_t result;
    struct nu_spi *spi_bus = (struct nu_spi *)pvUserData;

    RT_ASSERT(spi_bus);

    result = rt_sem_release(spi_bus->m_psSemBus);
    RT_ASSERT(result == RT_EOK);
}

static void nu_pdma_spi_tx_cb_trigger(void *pvUserData, uint32_t u32UserData)
{
    /* Get base address of spi register */
    LPSPI_T *spi_base = (LPSPI_T *)pvUserData;

    /* Trigger TX/RX PDMA transfer. */
    LPSPI_TRIGGER_TX_RX_PDMA(spi_base);
}

static void nu_pdma_spi_rx_cb_disable(void *pvUserData, uint32_t u32UserData)
{
    /* Get base address of spi register */
    LPSPI_T *spi_base = (LPSPI_T *)pvUserData;

    /* Stop TX/RX DMA transfer. */
    LPSPI_DISABLE_TX_RX_PDMA(spi_base);
}

static rt_err_t nu_pdma_spi_rx_config(struct nu_spi *spi_bus, uint8_t *pu8Buf, int32_t i32RcvLen, uint8_t bytes_per_word)
{
    struct nu_pdma_chn_cb sChnCB;

    rt_err_t result;
    rt_uint8_t *dst_addr = NULL;
    nu_pdma_memctrl_t memctrl = eMemCtl_Undefined;

    /* Get base address of spi register */
    LPSPI_T *spi_base = (LPSPI_T *)spi_bus->spi_base;

    rt_uint8_t spi_pdma_rx_chid = spi_bus->pdma_chanid_rx;

    nu_pdma_filtering_set(spi_pdma_rx_chid, NU_PDMA_EVENT_TRANSFER_DONE);

    /* Register ISR callback function */
    sChnCB.m_eCBType = eCBType_Event;
    sChnCB.m_pfnCBHandler = nu_pdma_spi_rx_cb_event;
    sChnCB.m_pvUserData = (void *)spi_bus;
    result = nu_pdma_callback_register(spi_pdma_rx_chid, &sChnCB);
    if (result != RT_EOK)
    {
        goto exit_nu_pdma_spi_rx_config;
    }

    /* Register Disable engine dma trigger callback function */
    sChnCB.m_eCBType = eCBType_Disable;
    sChnCB.m_pfnCBHandler = nu_pdma_spi_rx_cb_disable;
    sChnCB.m_pvUserData = (void *)spi_base;
    result = nu_pdma_callback_register(spi_pdma_rx_chid, &sChnCB);
    if (result != RT_EOK)
    {
        goto exit_nu_pdma_spi_rx_config;
    }

    if (pu8Buf == RT_NULL)
    {
        memctrl  = eMemCtl_SrcFix_DstFix;
        dst_addr = (rt_uint8_t *) &spi_bus->dummy;
    }
    else
    {
        memctrl  = eMemCtl_SrcFix_DstInc;
        dst_addr = pu8Buf;
    }

    result = nu_pdma_channel_memctrl_set(spi_pdma_rx_chid, memctrl);
    if (result != RT_EOK)
    {
        goto exit_nu_pdma_spi_rx_config;
    }

    result = nu_pdma_transfer(spi_pdma_rx_chid,
                              bytes_per_word * 8,
                              (uint32_t)&spi_base->RX,
                              (uint32_t)dst_addr,
                              i32RcvLen / bytes_per_word,
                              0);
exit_nu_pdma_spi_rx_config:

    return result;
}

static rt_err_t nu_pdma_spi_tx_config(struct nu_spi *spi_bus, const uint8_t *pu8Buf, int32_t i32SndLen, uint8_t bytes_per_word)
{
    struct nu_pdma_chn_cb sChnCB;

    rt_err_t result;
    rt_uint8_t *src_addr = NULL;
    nu_pdma_memctrl_t memctrl = eMemCtl_Undefined;

    /* Get base address of spi register */
    LPSPI_T *spi_base = (LPSPI_T *)spi_bus->spi_base;

    rt_uint8_t spi_pdma_tx_chid = spi_bus->pdma_chanid_tx;

    if (pu8Buf == RT_NULL)
    {
        spi_bus->dummy = 0;
        memctrl = eMemCtl_SrcFix_DstFix;
        src_addr = (rt_uint8_t *)&spi_bus->dummy;
    }
    else
    {
        memctrl = eMemCtl_SrcInc_DstFix;
        src_addr = (rt_uint8_t *)pu8Buf;
    }

    /* Register Disable engine dma trigger callback function */
    sChnCB.m_eCBType = eCBType_Trigger;
    sChnCB.m_pfnCBHandler = nu_pdma_spi_tx_cb_trigger;
    sChnCB.m_pvUserData = (void *)spi_base;
    result = nu_pdma_callback_register(spi_pdma_tx_chid, &sChnCB);
    if (result != RT_EOK)
    {
        goto exit_nu_pdma_spi_tx_config;
    }

    result = nu_pdma_channel_memctrl_set(spi_pdma_tx_chid, memctrl);
    if (result != RT_EOK)
    {
        goto exit_nu_pdma_spi_tx_config;
    }

    result = nu_pdma_transfer(spi_pdma_tx_chid,
                              bytes_per_word * 8,
                              (uint32_t)src_addr,
                              (uint32_t)&spi_base->TX,
                              i32SndLen / bytes_per_word,
                              0);
exit_nu_pdma_spi_tx_config:

    return result;
}


/**
 * SPI PDMA transfer
 */
static rt_size_t nu_spi_pdma_transmit(struct nu_spi *spi_bus, const uint8_t *send_addr, uint8_t *recv_addr, int length, uint8_t bytes_per_word)
{
    rt_err_t result = RT_EOK;

    result = nu_pdma_spi_rx_config(spi_bus, recv_addr, length, bytes_per_word);
    RT_ASSERT(result == RT_EOK);

    result = nu_pdma_spi_tx_config(spi_bus, send_addr, length, bytes_per_word);
    RT_ASSERT(result == RT_EOK);

    /* Wait RX-PDMA transfer done */
    result = rt_sem_take(spi_bus->m_psSemBus, RT_WAITING_FOREVER);
    RT_ASSERT(result == RT_EOK);

    return length;
}

#endif /* #if defined(BSP_USING_SPI_PDMA) */

static int nu_spi_read(LPSPI_T *spi_base, uint8_t *recv_addr, uint8_t bytes_per_word)
{
    int size = 0;

    // Read RX data
    if (!LPSPI_GET_RX_FIFO_EMPTY_FLAG(spi_base))
    {
        uint32_t val;
        // Read data from SPI RX FIFO
        switch (bytes_per_word)
        {
        case 4:
            val = LPSPI_READ_RX(spi_base);
            nu_set32_le(recv_addr, val);
            break;
        case 3:
            val = LPSPI_READ_RX(spi_base);
            nu_set24_le(recv_addr, val);
            break;
        case 2:
            val = LPSPI_READ_RX(spi_base);
            nu_set16_le(recv_addr, val);
            break;
        case 1:
            *recv_addr = LPSPI_READ_RX(spi_base);
            break;
        default:
            LOG_E("Data length is not supported.\n");
            break;
        }
        size = bytes_per_word;
    }
    return size;
}

static int nu_spi_write(LPSPI_T *spi_base, const uint8_t *send_addr, uint8_t bytes_per_word)
{
    // Wait SPI TX send data
    while (LPSPI_GET_TX_FIFO_FULL_FLAG(spi_base));

    // Input data to SPI TX
    switch (bytes_per_word)
    {
    case 4:
        LPSPI_WRITE_TX(spi_base, nu_get32_le(send_addr));
        break;
    case 3:
        LPSPI_WRITE_TX(spi_base, nu_get24_le(send_addr));
        break;
    case 2:
        LPSPI_WRITE_TX(spi_base, nu_get16_le(send_addr));
        break;
    case 1:
        LPSPI_WRITE_TX(spi_base, *((uint8_t *)send_addr));
        break;
    default:
        LOG_E("Data length is not supported.\n");
        break;
    }

    return bytes_per_word;
}

/**
 * @brief SPI bus polling
 * @param dev : The pointer of the specified SPI module.
 * @param send_addr : Source address
 * @param recv_addr : Destination address
 * @param length    : Data length
 */
static void nu_spi_transmission_with_poll(struct nu_spi *spi_bus,
        uint8_t *send_addr, uint8_t *recv_addr, int length, uint8_t bytes_per_word)
{
    LPSPI_T *spi_base = (LPSPI_T *)spi_bus->spi_base;

    // Write-only
    if ((send_addr != RT_NULL) && (recv_addr == RT_NULL))
    {
        while (length > 0)
        {
            send_addr += nu_spi_write(spi_base, send_addr, bytes_per_word);
            length -= bytes_per_word;
        }
    } // if (send_addr != RT_NULL && recv_addr == RT_NULL)
    // Read-only
    else if ((send_addr == RT_NULL) && (recv_addr != RT_NULL))
    {
        spi_bus->dummy = 0;
        while (length > 0)
        {
            /* Input data to SPI TX FIFO */
            length -= nu_spi_write(spi_base, (const uint8_t *)&spi_bus->dummy, bytes_per_word);

            /* Read data from RX FIFO */
            recv_addr += nu_spi_read(spi_base, recv_addr, bytes_per_word);
        }
    } // else if (send_addr == RT_NULL && recv_addr != RT_NULL)
    // Read&Write
    else
    {
        while (length > 0)
        {
            /* Input data to SPI TX FIFO */
            send_addr += nu_spi_write(spi_base, send_addr, bytes_per_word);
            length -= bytes_per_word;

            /* Read data from RX FIFO */
            recv_addr += nu_spi_read(spi_base, recv_addr, bytes_per_word);
        }
    } // else

    /* Wait RX or drain RX-FIFO */
    if (recv_addr)
    {
        // Wait SPI transmission done
        while (LPSPI_IS_BUSY(spi_base))
        {
            while (!LPSPI_GET_RX_FIFO_EMPTY_FLAG(spi_base))
            {
                recv_addr += nu_spi_read(spi_base, recv_addr, bytes_per_word);
            }
        }

        while (!LPSPI_GET_RX_FIFO_EMPTY_FLAG(spi_base))
        {
            recv_addr += nu_spi_read(spi_base, recv_addr, bytes_per_word);
        }
    }
    else
    {
        /* Clear SPI RX FIFO */
        nu_spi_drain_rxfifo(spi_bus->spi_base);
    }
}

static rt_uint32_t nu_spi_bus_xfer(struct rt_spi_device *device, struct rt_spi_message *message)
{
    struct nu_spi *spi_bus;
    struct rt_spi_configuration *configuration;
    uint8_t bytes_per_word;
    void *pvUserData;

    RT_ASSERT(device != RT_NULL);
    RT_ASSERT(device->bus != RT_NULL);
    RT_ASSERT(message != RT_NULL);

    spi_bus = (struct nu_spi *) device->bus;
    configuration = (struct rt_spi_configuration *)&spi_bus->configuration;
    bytes_per_word = configuration->data_width / 8;
    pvUserData = device->parent.user_data;

    if ((message->length % bytes_per_word) != 0)
    {
        /* Say bye. */
        LOG_E("%s: error payload length(%d%%%d != 0).\n", spi_bus->name, message->length, bytes_per_word);
        return 0;
    }

    if (message->length > 0)
    {
        if (message->cs_take && !(configuration->mode & RT_SPI_NO_CS))
        {
            if (pvUserData != RT_NULL)
            {
                if (configuration->mode & RT_SPI_CS_HIGH)
                {
                    // set to HIGH */
                    rt_pin_write(*((rt_base_t *)pvUserData), PIN_HIGH);
                }
                else
                {
                    // set to LOW */
                    rt_pin_write(*((rt_base_t *)pvUserData), PIN_LOW);
                }
            }
            else
            {
                if (configuration->mode & RT_SPI_CS_HIGH)
                {
                    LPSPI_SET_SS_HIGH(spi_bus->spi_base);
                }
                else
                {
                    LPSPI_SET_SS_LOW(spi_bus->spi_base);
                }
            }
        }

        nu_spi_transfer(spi_bus, (uint8_t *)message->send_buf, (uint8_t *)message->recv_buf, message->length, bytes_per_word);

        if (message->cs_release && !(configuration->mode & RT_SPI_NO_CS))
        {
            if (pvUserData != RT_NULL)
            {
                if (configuration->mode & RT_SPI_CS_HIGH)
                {
                    // set to LOW */
                    rt_pin_write(*((rt_base_t *)pvUserData), PIN_LOW);
                }
                else
                {
                    // set to HIGH */
                    rt_pin_write(*((rt_base_t *)pvUserData), PIN_HIGH);
                }
            }
            else
            {
                if (configuration->mode & RT_SPI_CS_HIGH)
                {
                    LPSPI_SET_SS_LOW(spi_bus->spi_base);
                }
                else
                {
                    LPSPI_SET_SS_HIGH(spi_bus->spi_base);
                }
            }
        }

    }

    return message->length;
}

static int nu_spi_register_bus(struct nu_spi *spi_bus, const char *name)
{
    return rt_spi_bus_register(&spi_bus->dev, name, &nu_spi_poll_ops);
}

/**
 * Hardware SPI Initial
 */
static int rt_hw_lpspi_init(void)
{
    int i;

    for (i = (LPSPI_START + 1); i < LPSPI_CNT; i++)
    {
        SYS_ResetModule(nu_spi_arr[i].rstidx);
        nu_spi_register_bus(&nu_spi_arr[i], nu_spi_arr[i].name);
#if defined(BSP_USING_SPI_PDMA)
        nu_spi_arr[i].pdma_chanid_tx = -1;
        nu_spi_arr[i].pdma_chanid_rx = -1;
        if ((nu_spi_arr[i].pdma_perp_tx != NU_PDMA_UNUSED) && (nu_spi_arr[i].pdma_perp_rx != NU_PDMA_UNUSED))
        {
            if (nu_hw_spi_pdma_allocate(&nu_spi_arr[i]) != RT_EOK)
            {
                LOG_W("Failed to allocate DMA channels for %s. We will use poll-mode for this bus.\n", nu_spi_arr[i].name);
            }
        }
#endif
    }

    return 0;
}

INIT_DEVICE_EXPORT(rt_hw_lpspi_init);

#endif //#if defined(BSP_USING_LPSPI)
