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

#if defined(BSP_USING_LPUART)

#include <rtdevice.h>
#include <rthw.h>
#include "NuMicro.h"
#include <drv_uart.h>

#if defined(RT_SERIAL_USING_DMA)
    #include <drv_pdma.h>
#endif

#define LOG_TAG    "drv.uart"
#undef  DBG_ENABLE
#define DBG_SECTION_NAME   LOG_TAG
#define DBG_LEVEL      LOG_LVL_WARNING
#define DBG_COLOR
#include <rtdbg.h>

/* Private define ---------------------------------------------------------------*/
enum
{
    LPUART_START = -1,
#if defined(BSP_USING_LPUART0)
    LPUART0_IDX,
#endif
    LPUART_CNT
};

//#undef BSP_USING_MMU

struct nu_rxbuf_ctx
{
    uint8_t *pu8RxBuf;
    uint32_t bufsize;
    uint32_t put_index;
    uint32_t reserved;
};
typedef struct nu_rxbuf_ctx *nu_rxbuf_ctx_t;

/* Private typedef --------------------------------------------------------------*/
struct nu_uart
{
    rt_serial_t dev;
    char *name;
    LPUART_T *base;
    uint32_t rst;
    IRQn_Type irqn;

#if defined(RT_SERIAL_USING_DMA)
    uint32_t dma_flag;
    uint32_t pdma_perp_tx;
    uint32_t pdma_chanid_tx;

    uint32_t pdma_perp_rx;
    uint32_t pdma_chanid_rx;

    nu_pdma_desc_t pdma_rx_desc;
    struct nu_rxbuf_ctx dmabuf;
#endif

};
typedef struct nu_uart *nu_uart_t;

/* Private functions ------------------------------------------------------------*/
static rt_err_t nu_uart_configure(struct rt_serial_device *serial, struct serial_configure *cfg);
static rt_err_t nu_uart_control(struct rt_serial_device *serial, int cmd, void *arg);
static int nu_uart_send(struct rt_serial_device *serial, char c);
static int nu_uart_receive(struct rt_serial_device *serial);
static void nu_uart_isr(nu_uart_t serial);

#if defined(RT_SERIAL_USING_DMA)
    static rt_size_t nu_uart_dma_transmit(struct rt_serial_device *serial, rt_uint8_t *buf, rt_size_t size, int direction);
    static void nu_pdma_uart_rx_cb(void *pvOwner, uint32_t u32Events);
    static void nu_pdma_uart_tx_cb(void *pvOwner, uint32_t u32Events);
#endif

/* Public functions ------------------------------------------------------------*/

/* Private variables ------------------------------------------------------------*/

static const struct rt_uart_ops nu_uart_ops =
{
    .configure = nu_uart_configure,
    .control = nu_uart_control,
    .putc = nu_uart_send,
    .getc = nu_uart_receive,
#if defined(RT_SERIAL_USING_DMA)
    .dma_transmit = nu_uart_dma_transmit
#else
    .dma_transmit = RT_NULL
#endif
};

static const struct serial_configure nu_uart_default_config =
        RT_SERIAL_CONFIG_DEFAULT;

static struct nu_uart nu_uart_arr [] =
{
#if defined(BSP_USING_LPUART0)
    {
        .name = "lpuart0",
        .base = LPUART0,
        .rst = LPUART0_RST,
        .irqn = LPUART0_IRQn,
#if defined(RT_SERIAL_USING_DMA)
#if defined(BSP_USING_LPUART0_TX_DMA)
        .pdma_perp_tx = NU_PDMA_SET_REQ_SRC_ID(LPPDMA_IDX, LPPDMA_LPUART0_TX),
#else
        .pdma_perp_tx = NU_PDMA_UNUSED,
#endif
#if defined(BSP_USING_LPUART0_RX_DMA)
        .pdma_perp_rx = NU_PDMA_SET_REQ_SRC_ID(LPPDMA_IDX, LPPDMA_LPUART0_RX),
#else
        .pdma_perp_rx = NU_PDMA_UNUSED,
#endif
#endif
    },
#endif

}; /* uart nu_uart */

/* Interrupt Handle Function  ----------------------------------------------------*/
#if defined(BSP_USING_LPUART0)
/* LPUART0 interrupt entry */
void LPUART0_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    nu_uart_isr(&nu_uart_arr[LPUART0_IDX]);

    /* leave interrupt */
    rt_interrupt_leave();
}
#endif

/**
 * All UART interrupt service routine
 */
static void nu_uart_isr(nu_uart_t psNuUart)
{
    /* Get base address of uart register */
    LPUART_T *base = psNuUart->base;

    /* Get interrupt event */
    uint32_t u32IntSts = base->INTSTS;
    uint32_t u32FIFOSts = base->FIFOSTS;

#if defined(RT_SERIAL_USING_DMA)
    if (u32IntSts & LPUART_INTSTS_HWRLSIF_Msk)
    {
        /* Drain RX FIFO to remove remain FEF frames in FIFO. */
        base->FIFO |= LPUART_FIFO_RXRST_Msk;
        base->FIFOSTS |= (LPUART_FIFOSTS_BIF_Msk | LPUART_FIFOSTS_FEF_Msk | LPUART_FIFOSTS_PEF_Msk);
        return;
    }
#endif

    /* Handle RX event */
    if (u32IntSts & (LPUART_INTSTS_RDAINT_Msk | LPUART_INTSTS_RXTOINT_Msk))
    {
        rt_hw_serial_isr(&psNuUart->dev, RT_SERIAL_EVENT_RX_IND);
    }
    base->INTSTS = u32IntSts;
    base->FIFOSTS = u32FIFOSts;
}

/**
 * Set RS-485 AUD mode
 */
void nu_lpuart_set_rs485aud(struct rt_serial_device *serial, rt_bool_t bRTSActiveLowLevel)
{
    nu_uart_t psNuUart = (nu_uart_t)serial;
    LPUART_T *base;
    RT_ASSERT(serial);

    /* Get base address of uart register */
    base = ((nu_uart_t)serial)->base;

    /* Set RTS as RS-485 phy direction controlling ping. */
    LPUART_SelectRS485Mode(base, LPUART_ALTCTL_RS485AUD_Msk, 0);

    if (bRTSActiveLowLevel)
    {
        /* Set direction pin as active-low. */
        base->MODEM |= LPUART_MODEM_RTSACTLV_Msk;
    }
    else
    {
        /* Set direction pin as active-high. */
        base->MODEM &= ~LPUART_MODEM_RTSACTLV_Msk;
    }

    LOG_I("Set %s to RS-485 AUD function mode. ActiveLowLevel-%s", psNuUart->name, bRTSActiveLowLevel ? "YES" : "NO");
}

/**
 * Configure uart port
 */
static rt_err_t nu_uart_configure(struct rt_serial_device *serial, struct serial_configure *cfg)
{
    nu_uart_t psNuUart = (nu_uart_t)serial;
    rt_err_t ret = RT_EOK;
    uint32_t uart_word_len, uart_stop_bit, uart_parity;

    RT_ASSERT(serial);
    RT_ASSERT(cfg);

    /* Check baudrate */
    RT_ASSERT(cfg->baud_rate != 0);

    uart_word_len = uart_stop_bit = uart_parity = 0;

    /* Get base address of uart register */
    LPUART_T *base = psNuUart->base;

    /* Check word len */
    switch (cfg->data_bits)
    {
    case DATA_BITS_5:
        uart_word_len = LPUART_WORD_LEN_5;
        break;

    case DATA_BITS_6:
        uart_word_len = LPUART_WORD_LEN_6;
        break;

    case DATA_BITS_7:
        uart_word_len = LPUART_WORD_LEN_7;
        break;

    case DATA_BITS_8:
        uart_word_len = LPUART_WORD_LEN_8;
        break;

    default:
        LOG_E("Unsupported data length.");
        ret = RT_EINVAL;
        goto exit_nu_uart_configure;
    }

    /* Check stop bit */
    switch (cfg->stop_bits)
    {
    case STOP_BITS_1:
        uart_stop_bit = LPUART_STOP_BIT_1;
        break;

    case STOP_BITS_2:
        uart_stop_bit = LPUART_STOP_BIT_2;
        break;

    default:
        LOG_E("Unsupported stop bit.");
        ret = RT_EINVAL;
        goto exit_nu_uart_configure;
    }

    /* Check parity */
    switch (cfg->parity)
    {
    case PARITY_NONE:
        uart_parity = LPUART_PARITY_NONE;
        break;

    case PARITY_ODD:
        uart_parity = LPUART_PARITY_ODD;
        break;

    case PARITY_EVEN:
        uart_parity = LPUART_PARITY_EVEN;
        break;

    default:
        LOG_E("Unsupported parity.");
        ret = RT_EINVAL;
        goto exit_nu_uart_configure;
    }

    /* Reset this module */
    SYS_ResetModule(psNuUart->rst);

    /* Open Uart and set UART Baudrate */
    LPUART_Open(base, cfg->baud_rate);

    /* Set line configuration. */
    LPUART_SetLine_Config(base, 0, uart_word_len, uart_parity, uart_stop_bit);

    /* Enable NVIC interrupt. */
    NVIC_EnableIRQ(psNuUart->irqn);

exit_nu_uart_configure:

    if (ret != RT_EOK)
        LPUART_Close(base);

    return -(ret);
}

#if defined(RT_SERIAL_USING_DMA)

static void nu_pdma_uart_rxbuf_free(nu_uart_t psNuUart)
{
    struct rt_serial_device *serial = (struct rt_serial_device *)psNuUart;

    if ((serial->config.bufsz > 0) && psNuUart->dmabuf.pu8RxBuf)
        rt_free_align(psNuUart->dmabuf.pu8RxBuf);

    psNuUart->dmabuf.pu8RxBuf = RT_NULL;
    psNuUart->dmabuf.bufsize = 0;
    psNuUart->dmabuf.put_index = 0;
}

static rt_err_t nu_pdma_uart_rx_config(nu_uart_t psNuUart, uint8_t *pu8Buf, int32_t i32TriggerLen)
{
    struct rt_serial_device *serial = (struct rt_serial_device *)psNuUart;

    rt_err_t result = RT_EOK;
    struct nu_pdma_chn_cb sChnCB;

    uint32_t u32IdleTimeoutInUs = 1500;

    /* Get base address of uart register */
    LPUART_T *base = psNuUart->base;

    /* Register ISR callback function */
    sChnCB.m_eCBType = eCBType_Event;
    sChnCB.m_pfnCBHandler = nu_pdma_uart_rx_cb;
    sChnCB.m_pvUserData = (void *)psNuUart;

    nu_pdma_filtering_set(psNuUart->pdma_chanid_rx, NU_PDMA_EVENT_TRANSFER_DONE | NU_PDMA_EVENT_TIMEOUT);
    result = nu_pdma_callback_register(psNuUart->pdma_chanid_rx, &sChnCB);
    if (result != RT_EOK)
    {
        goto exit_nu_pdma_uart_rx_config;
    }

#define DEF_WAIT_TIME   10   // 10 word
    if (psNuUart->dev.config.baud_rate > 0)
    {
        u32IdleTimeoutInUs = 1000000 * DEF_WAIT_TIME * (1 + psNuUart->dev.config.data_bits + (psNuUart->dev.config.stop_bits + 1)) / psNuUart->dev.config.baud_rate;
    }

    LOG_I("[%s] Set idle time out %d us", psNuUart->name, u32IdleTimeoutInUs);
    LOG_I("[%s] bufsz: %d B", psNuUart->name, serial->config.bufsz);

    if (serial->config.bufsz == 0)
    {
        psNuUart->dmabuf.bufsize = i32TriggerLen;
        psNuUart->dmabuf.put_index = 0;
        psNuUart->dmabuf.pu8RxBuf = pu8Buf;

        rt_kprintf("pu8Buf=%08x, i32TriggerLen=%d\n", pu8Buf, i32TriggerLen);

        result = nu_pdma_transfer(psNuUart->pdma_chanid_rx,
                                  8,
                                  (uint32_t)&base->DAT,
                                  (uint32_t)pu8Buf,
                                  i32TriggerLen,
                                  u32IdleTimeoutInUs);  //Idle-timeout
        if (result != RT_EOK)
        {
            goto exit_nu_pdma_uart_rx_config;
        }
    }
    else
    {
        struct rt_serial_rx_fifo *rx_fifo = (struct rt_serial_rx_fifo *)serial->serial_rx;
        psNuUart->dmabuf.bufsize = i32TriggerLen;
        psNuUart->dmabuf.put_index = 0;

        /* For Serial RX FIFO - Single buffer recycle SG trigger */
        result = nu_pdma_desc_setup(psNuUart->pdma_chanid_rx,
                                    psNuUart->pdma_rx_desc,
                                    8,
                                    (uint32_t)&base->DAT,
                                    (uint32_t)rx_fifo->buffer,
                                    i32TriggerLen,
                                    psNuUart->pdma_rx_desc,
                                    0);
        if (result != RT_EOK)
        {
            goto exit_nu_pdma_uart_rx_config;
        }

        /* Assign head descriptor & go */
        result = nu_pdma_sg_transfer(psNuUart->pdma_chanid_rx, psNuUart->pdma_rx_desc, u32IdleTimeoutInUs);
        if (result != RT_EOK)
        {
            goto exit_nu_pdma_uart_rx_config;
        }
    }

    /* Enable Receive Line interrupt & Start DMA RX transfer. */
    LPUART_ENABLE_INT(base, LPUART_INTEN_RLSIEN_Msk | LPUART_INTEN_RXPDMAEN_Msk);

exit_nu_pdma_uart_rx_config:

    return result;
}

static void nu_pdma_uart_rx_cb(void *pvOwner, uint32_t u32Events)
{
    nu_uart_t psNuUart = (nu_uart_t)pvOwner;
    struct rt_serial_device *serial = (struct rt_serial_device *)psNuUart;
    rt_size_t recv_len = 0;
    uint32_t dma_put_index = 0;

    RT_ASSERT(psNuUart);

    /* Get base address of uart register */
    LPUART_T *base = psNuUart->base;
    nu_rxbuf_ctx_t psNuRxBufCtx = &psNuUart->dmabuf;

    dma_put_index = nu_pdma_transferred_byte_get(psNuUart->pdma_chanid_rx, psNuRxBufCtx->bufsize);
    if (u32Events & (NU_PDMA_EVENT_TRANSFER_DONE | NU_PDMA_EVENT_TIMEOUT))
    {
        if (u32Events & NU_PDMA_EVENT_TRANSFER_DONE)
        {
            dma_put_index = psNuRxBufCtx->bufsize;
        }
        else if ((u32Events & NU_PDMA_EVENT_TIMEOUT) && !LPUART_GET_RX_EMPTY(base))
        {
            return;
        }

        recv_len = dma_put_index - psNuRxBufCtx->put_index;

        if (recv_len > 0)
        {
            psNuRxBufCtx->put_index = dma_put_index % psNuRxBufCtx->bufsize;
        }
    }

    if ((serial->config.bufsz == 0) && (u32Events & NU_PDMA_EVENT_TRANSFER_DONE))
    {
        recv_len = psNuRxBufCtx->bufsize;
    }

    if (recv_len > 0)
    {
        rt_hw_serial_isr(&psNuUart->dev, RT_SERIAL_EVENT_RX_DMADONE | (recv_len << 8));
    }
}

static rt_err_t nu_pdma_uart_tx_config(nu_uart_t psNuUart)
{
    struct nu_pdma_chn_cb sChnCB;
    RT_ASSERT(psNuUart);

    /* Register ISR callback function */
    sChnCB.m_eCBType = eCBType_Event;
    sChnCB.m_pfnCBHandler = nu_pdma_uart_tx_cb;
    sChnCB.m_pvUserData = (void *)psNuUart;

    nu_pdma_filtering_set(psNuUart->pdma_chanid_tx, NU_PDMA_EVENT_TRANSFER_DONE);
    return nu_pdma_callback_register(psNuUart->pdma_chanid_tx, &sChnCB);
}

static void nu_pdma_uart_tx_cb(void *pvOwner, uint32_t u32Events)
{
    nu_uart_t psNuUart = (nu_uart_t)pvOwner;
    RT_ASSERT(psNuUart);

    LPUART_DISABLE_INT(psNuUart->base, LPUART_INTEN_TXPDMAEN_Msk);// Stop DMA TX transfer

    if (u32Events & NU_PDMA_EVENT_TRANSFER_DONE)
    {
        /* Get base address of uart register */
        LPUART_T *base = psNuUart->base;

        /* Waiting if TX-FIFO is empty. */
        while (!(LPUART_IS_TX_EMPTY(base)));

        rt_hw_serial_isr(&psNuUart->dev, RT_SERIAL_EVENT_TX_DMADONE);
    }
}

/**
 * Uart DMA transfer
 */
static rt_size_t nu_uart_dma_transmit(struct rt_serial_device *serial, rt_uint8_t *buf, rt_size_t size, int direction)
{
    rt_err_t result = RT_EOK;
    nu_uart_t psNuUart = (nu_uart_t)serial;

    RT_ASSERT(serial);
    RT_ASSERT(buf);

    /* Get base address of uart register */
    LPUART_T *base = psNuUart->base;
    if (direction == RT_SERIAL_DMA_TX)
    {
        result = nu_pdma_transfer(psNuUart->pdma_chanid_tx,
                                  8,
                                  (uint32_t)buf,
                                  (uint32_t)base,
                                  size,
                                  0);  // wait-forever
        // Start DMA TX transfer
        LPUART_ENABLE_INT(base, LPUART_INTEN_TXPDMAEN_Msk);
    }
    else if (direction == RT_SERIAL_DMA_RX)
    {
        LPUART_DISABLE_INT(base, LPUART_INTEN_RLSIEN_Msk | LPUART_INTEN_RXPDMAEN_Msk | LPUART_INTEN_TOCNTEN_Msk);

        // If config.bufsz = 0, serial will trigger once.
        result = nu_pdma_uart_rx_config(psNuUart, buf, size);
    }
    else
    {
        result = RT_ERROR;
    }

    return result;
}

static int nu_hw_uart_dma_allocate(nu_uart_t psNuUart)
{
    RT_ASSERT(psNuUart);

    /* Allocate LPUART_TX nu_dma channel */
    if (psNuUart->pdma_perp_tx != NU_PDMA_UNUSED)
    {
        psNuUart->pdma_chanid_tx = nu_pdma_channel_allocate(psNuUart->pdma_perp_tx);
        if (psNuUart->pdma_chanid_tx >= 0)
        {
            psNuUart->dma_flag |= RT_DEVICE_FLAG_DMA_TX;
        }
    }

    /* Allocate LPUART_RX nu_dma channel */
    if (psNuUart->pdma_perp_rx != NU_PDMA_UNUSED)
    {
        psNuUart->pdma_chanid_rx = nu_pdma_channel_allocate(psNuUart->pdma_perp_rx);
        if (psNuUart->pdma_chanid_rx >= 0)
        {
            rt_err_t ret = RT_EOK;
            psNuUart->dma_flag |= RT_DEVICE_FLAG_DMA_RX;
            ret = nu_pdma_sgtbls_allocate(psNuUart->pdma_chanid_rx, &psNuUart->pdma_rx_desc, 1);
            RT_ASSERT(ret == RT_EOK);
        }
    }

    return RT_EOK;
}
#endif

/**
 * Uart interrupt control
 */
static rt_err_t nu_uart_control(struct rt_serial_device *serial, int cmd, void *arg)
{
    nu_uart_t psNuUart = (nu_uart_t)serial;
    rt_err_t result = RT_EOK;
    rt_ubase_t ctrl_arg = (rt_ubase_t)arg;

    RT_ASSERT(serial);

    /* Get base address of uart register */
    LPUART_T *base = psNuUart->base;

    switch (cmd)
    {
    case RT_DEVICE_CTRL_CLR_INT:
        if (ctrl_arg == RT_DEVICE_FLAG_INT_RX) /* Disable INT-RX */
        {
            LPUART_DISABLE_INT(base, LPUART_INTEN_RDAIEN_Msk | LPUART_INTEN_RXTOIEN_Msk | LPUART_INTEN_TOCNTEN_Msk);
        }
        else if (ctrl_arg == RT_DEVICE_FLAG_DMA_RX) /* Disable DMA-RX */
        {
            /* Disable Receive Line interrupt & Stop DMA RX transfer. */
#if defined(RT_SERIAL_USING_DMA)
            if (psNuUart->dma_flag & RT_DEVICE_FLAG_DMA_RX)
            {
                nu_pdma_channel_terminate(psNuUart->pdma_chanid_rx);
                nu_pdma_uart_rxbuf_free(psNuUart);
            }
            LPUART_DISABLE_INT(base, LPUART_INTEN_RLSIEN_Msk | LPUART_INTEN_RXPDMAEN_Msk | LPUART_INTEN_TOCNTEN_Msk);
#endif
        }
        break;

    case RT_DEVICE_CTRL_SET_INT:
        if (ctrl_arg == RT_DEVICE_FLAG_INT_RX) /* Enable INT-RX */
        {
            LPUART_ENABLE_INT(base, LPUART_INTEN_RDAIEN_Msk | LPUART_INTEN_RXTOIEN_Msk | LPUART_INTEN_TOCNTEN_Msk);
        }
        break;

#if defined(RT_SERIAL_USING_DMA)
    case RT_DEVICE_CTRL_CONFIG:
        if (ctrl_arg == RT_DEVICE_FLAG_DMA_RX) /* Configure and trigger DMA-RX */
        {
            struct rt_serial_rx_fifo *rx_fifo = (struct rt_serial_rx_fifo *)serial->serial_rx;
            result = nu_pdma_uart_rx_config(psNuUart, rx_fifo->buffer, serial->config.bufsz);  // Config & trigger
        }
        else if (ctrl_arg == RT_DEVICE_FLAG_DMA_TX) /* Configure DMA-TX */
        {
            result = nu_pdma_uart_tx_config(psNuUart);
        }
        break;
#endif

    case RT_DEVICE_CTRL_CLOSE:
        /* Disable NVIC interrupt. */
        NVIC_DisableIRQ(psNuUart->irqn);

#if defined(RT_SERIAL_USING_DMA)
        LPUART_DISABLE_INT(base, LPUART_INTEN_RLSIEN_Msk | LPUART_INTEN_RXPDMAEN_Msk);
        LPUART_DISABLE_INT(base, LPUART_INTEN_TXPDMAEN_Msk);

        if (psNuUart->dma_flag != 0)
        {
            nu_pdma_channel_terminate(psNuUart->pdma_chanid_tx);
            nu_pdma_channel_terminate(psNuUart->pdma_chanid_rx);
            nu_pdma_uart_rxbuf_free(psNuUart);
        }
#endif

        /* Close UART port */
        LPUART_Close(base);

        break;

    default:
        result = -RT_EINVAL;
        break;

    }
    return result;
}

/**
 * Uart put char
 */
static int nu_uart_send(struct rt_serial_device *serial, char c)
{
    nu_uart_t psNuUart = (nu_uart_t)serial;

    RT_ASSERT(serial);

    /* Get base address of uart register */
    LPUART_T *base = psNuUart->base;

    /* Waiting if TX-FIFO is full. */
    while (LPUART_IS_TX_FULL(base));

    /* Put char into TX-FIFO */
    LPUART_WRITE(base, c);

    return 1;
}

/**
 * Uart get char
 */
static int nu_uart_receive(struct rt_serial_device *serial)
{
    nu_uart_t psNuUart = (nu_uart_t)serial;

    RT_ASSERT(serial);

    /* Get base address of uart register */
    LPUART_T *base = psNuUart->base;

    /* Return failure if RX-FIFO is empty. */
    if (LPUART_GET_RX_EMPTY(base))
    {
        return -1;
    }

    /* Get char from RX-FIFO */
    return LPUART_READ(base);
}

/**
 * Hardware UART Initialization
 */
static int rt_hw_lpuart_init(void)
{
    int i;
    rt_uint32_t flag;
    rt_err_t ret = RT_EOK;

    for (i = (LPUART_START + 1); i < LPUART_CNT; i++)
    {
        flag = RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX;

        nu_uart_arr[i].dev.ops    = &nu_uart_ops;
        nu_uart_arr[i].dev.config = nu_uart_default_config;

#if defined(RT_SERIAL_USING_DMA)
        nu_uart_arr[i].dma_flag = 0;
        nu_hw_uart_dma_allocate(&nu_uart_arr[i]);
        flag |= nu_uart_arr[i].dma_flag;
        rt_memset(&nu_uart_arr[i].dmabuf, 0, sizeof(struct nu_rxbuf_ctx));
#endif

        ret = rt_hw_serial_register(&nu_uart_arr[i].dev, nu_uart_arr[i].name, flag, NULL);
        RT_ASSERT(ret == RT_EOK);

#if defined(RT_USING_CONSOLE) && defined(RT_USING_DEVICE)
        rt_console_set_device(RT_CONSOLE_DEVICE_NAME);
#endif
    }

    return ret;
}
INIT_DEVICE_EXPORT(rt_hw_lpuart_init);

#endif //#if defined(BSP_USING_LPUART)
