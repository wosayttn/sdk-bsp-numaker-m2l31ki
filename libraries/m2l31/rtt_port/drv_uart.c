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

#if defined(BSP_USING_UART)

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
    UART_START = -1,
#if defined(BSP_USING_UART0)
    UART0_IDX,
#endif
#if defined(BSP_USING_UART1)
    UART1_IDX,
#endif
#if defined(BSP_USING_UART2)
    UART2_IDX,
#endif
#if defined(BSP_USING_UART3)
    UART3_IDX,
#endif
#if defined(BSP_USING_UART4)
    UART4_IDX,
#endif
#if defined(BSP_USING_UART5)
    UART5_IDX,
#endif
#if defined(BSP_USING_UART6)
    UART6_IDX,
#endif
#if defined(BSP_USING_UART7)
    UART7_IDX,
#endif
    UART_CNT
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
    UART_T *base;
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
#if defined(BSP_USING_UART0)
    {
        .name = "uart0",
        .base = UART0,
        .rst = UART0_RST,
        .irqn = UART0_IRQn,

#if defined(RT_SERIAL_USING_DMA)
#if defined(BSP_USING_UART0_TX_DMA)
        .pdma_perp_tx = PDMA_UART0_TX,
#else
        .pdma_perp_tx = NU_PDMA_UNUSED,
#endif
#if defined(BSP_USING_UART0_RX_DMA)
        .pdma_perp_rx = PDMA_UART0_RX,
#else
        .pdma_perp_rx = NU_PDMA_UNUSED,
#endif
#endif
    },
#endif

#if defined(BSP_USING_UART1)
    {
        .name = "uart1",
        .base = UART1,
        .rst = UART1_RST,
        .irqn = UART1_IRQn,
#if defined(RT_SERIAL_USING_DMA)
#if defined(BSP_USING_UART1_TX_DMA)
        .pdma_perp_tx = PDMA_UART1_TX,
#else
        .pdma_perp_tx = NU_PDMA_UNUSED,
#endif
#if defined(BSP_USING_UART1_RX_DMA)
        .pdma_perp_rx = PDMA_UART1_RX,
#else
        .pdma_perp_rx = NU_PDMA_UNUSED,
#endif
#endif
    },
#endif

#if defined(BSP_USING_UART2)
    {
        .name = "uart2",
        .base = UART2,
        .rst = UART2_RST,
        .irqn = UART2_IRQn,
#if defined(RT_SERIAL_USING_DMA)
#if defined(BSP_USING_UART2_TX_DMA)
        .pdma_perp_tx = PDMA_UART2_TX,
#else
        .pdma_perp_tx = NU_PDMA_UNUSED,
#endif
#if defined(BSP_USING_UART2_RX_DMA)
        .pdma_perp_rx = PDMA_UART2_RX,
#else
        .pdma_perp_rx = NU_PDMA_UNUSED,
#endif
#endif
    },
#endif

#if defined(BSP_USING_UART3)
    {
        .name = "uart3",
        .base = UART3,
        .rst = UART3_RST,
        .irqn = UART3_IRQn,
#if defined(RT_SERIAL_USING_DMA)
#if defined(BSP_USING_UART3_TX_DMA)
        .pdma_perp_tx = PDMA_UART3_TX,
#else
        .pdma_perp_tx = NU_PDMA_UNUSED,
#endif
#if defined(BSP_USING_UART3_RX_DMA)
        .pdma_perp_rx = PDMA_UART3_RX,
#else
        .pdma_perp_rx = NU_PDMA_UNUSED,
#endif
#endif
    },
#endif

#if defined(BSP_USING_UART4)
    {
        .name = "uart4",
        .base = UART4,
        .rst = UART4_RST,
        .irqn = UART4_IRQn,
#if defined(RT_SERIAL_USING_DMA)
#if defined(BSP_USING_UART4_TX_DMA)
        .pdma_perp_tx = PDMA_UART4_TX,
#else
        .pdma_perp_tx = NU_PDMA_UNUSED,
#endif
#if defined(BSP_USING_UART4_RX_DMA)
        .pdma_perp_rx = PDMA_UART4_RX,
#else
        .pdma_perp_rx = NU_PDMA_UNUSED,
#endif
#endif
    },
#endif

#if defined(BSP_USING_UART5)
    {
        .name = "uart5",
        .base = UART5,
        .rst = UART5_RST,
        .irqn = UART5_IRQn,
#if defined(RT_SERIAL_USING_DMA)
#if defined(BSP_USING_UART5_TX_DMA)
        .pdma_perp_tx = PDMA_UART5_TX,
#else
        .pdma_perp_tx = NU_PDMA_UNUSED,
#endif
#if defined(BSP_USING_UART5_RX_DMA)
        .pdma_perp_rx = PDMA_UART5_RX,
#else
        .pdma_perp_rx = NU_PDMA_UNUSED,
#endif
#endif
    },
#endif

#if defined(BSP_USING_UART6)
    {
        .name = "uart6",
        .base = UART6,
        .rst = UART6_RST,
        .irqn = UART6_IRQn,
#if defined(RT_SERIAL_USING_DMA)
#if defined(BSP_USING_UART6_TX_DMA)
        .pdma_perp_tx = PDMA_UART6_TX,
#else
        .pdma_perp_tx = NU_PDMA_UNUSED,
#endif
#if defined(BSP_USING_UART6_RX_DMA)
        .pdma_perp_rx = PDMA_UART6_RX,
#else
        .pdma_perp_rx = NU_PDMA_UNUSED,
#endif
#endif
    },
#endif

#if defined(BSP_USING_UART7)
    {
        .name = "uart7",
        .base = UART7,
        .rst = UART7_RST,
        .irqn = UART7_IRQn,
#if defined(RT_SERIAL_USING_DMA)
#if defined(BSP_USING_UART7_TX_DMA)
        .pdma_perp_tx = PDMA_UART7_TX,
#else
        .pdma_perp_tx = NU_PDMA_UNUSED,
#endif
#if defined(BSP_USING_UART7_RX_DMA)
        .pdma_perp_rx = PDMA_UART7_RX,
#else
        .pdma_perp_rx = NU_PDMA_UNUSED,
#endif
#endif
    },
#endif

}; /* uart nu_uart */

/* Interrupt Handle Function  ----------------------------------------------------*/
#if defined(BSP_USING_UART0)
/* UART0 interrupt entry */
void UART0_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    nu_uart_isr(&nu_uart_arr[UART0_IDX]);

    /* leave interrupt */
    rt_interrupt_leave();
}
#endif

#if defined(BSP_USING_UART1)
/* UART1 interrupt entry */
void UART1_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    nu_uart_isr(&nu_uart_arr[UART1_IDX]);

    /* leave interrupt */
    rt_interrupt_leave();
}
#endif

#if defined(BSP_USING_UART2)
/* UART2 interrupt entry */
void UART2_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    nu_uart_isr(&nu_uart_arr[UART2_IDX]);

    /* leave interrupt */
    rt_interrupt_leave();
}
#endif

#if defined(BSP_USING_UART3)
/* UART3 interrupt service routine */
void UART3_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    nu_uart_isr(&nu_uart_arr[UART3_IDX]);

    /* leave interrupt */
    rt_interrupt_leave();
}
#endif

#if defined(BSP_USING_UART4)
/* UART4 interrupt entry */
void UART4_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    nu_uart_isr(&nu_uart_arr[UART4_IDX]);

    /* leave interrupt */
    rt_interrupt_leave();
}
#endif

#if defined(BSP_USING_UART5)
/* UART5 interrupt entry */
void UART5_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    nu_uart_isr(&nu_uart_arr[UART5_IDX]);

    /* leave interrupt */
    rt_interrupt_leave();
}
#endif

#if defined(BSP_USING_UART6)
/* UART6 interrupt entry */
void UART6_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    nu_uart_isr(&nu_uart_arr[UART6_IDX]);

    /* leave interrupt */
    rt_interrupt_leave();
}
#endif

#if defined(BSP_USING_UART7)
/* UART7 interrupt entry */
void UART7_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    nu_uart_isr(&nu_uart_arr[UART7_IDX]);

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
    UART_T *base = psNuUart->base;

    /* Get interrupt event */
    uint32_t u32IntSts = base->INTSTS;
    uint32_t u32FIFOSts = base->FIFOSTS;

#if defined(RT_SERIAL_USING_DMA)
    if (u32IntSts & UART_INTSTS_HWRLSIF_Msk)
    {
        /* Drain RX FIFO to remove remain FEF frames in FIFO. */
        base->FIFO |= UART_FIFO_RXRST_Msk;
        base->FIFOSTS |= (UART_FIFOSTS_BIF_Msk | UART_FIFOSTS_FEF_Msk | UART_FIFOSTS_PEF_Msk);
        return;
    }
#endif

    /* Handle RX event */
    if (u32IntSts & (UART_INTSTS_RDAINT_Msk | UART_INTSTS_RXTOINT_Msk))
    {
        rt_hw_serial_isr(&psNuUart->dev, RT_SERIAL_EVENT_RX_IND);
    }
    base->INTSTS = u32IntSts;
    base->FIFOSTS = u32FIFOSts;
}

/**
 * Set RS-485 AUD mode
 */
void nu_uart_set_rs485aud(struct rt_serial_device *serial, rt_bool_t bRTSActiveLowLevel)
{
    nu_uart_t psNuUart = (nu_uart_t)serial;
    UART_T *base;
    RT_ASSERT(serial);

    /* Get base address of uart register */
    base = ((nu_uart_t)serial)->base;

    /* Set RTS as RS-485 phy direction controlling ping. */
    UART_SelectRS485Mode(base, UART_ALTCTL_RS485AUD_Msk, 0);

    if (bRTSActiveLowLevel)
    {
        /* Set direction pin as active-low. */
        base->MODEM |= UART_MODEM_RTSACTLV_Msk;
    }
    else
    {
        /* Set direction pin as active-high. */
        base->MODEM &= ~UART_MODEM_RTSACTLV_Msk;
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
    UART_T *base = psNuUart->base;

    /* Check word len */
    switch (cfg->data_bits)
    {
    case DATA_BITS_5:
        uart_word_len = UART_WORD_LEN_5;
        break;

    case DATA_BITS_6:
        uart_word_len = UART_WORD_LEN_6;
        break;

    case DATA_BITS_7:
        uart_word_len = UART_WORD_LEN_7;
        break;

    case DATA_BITS_8:
        uart_word_len = UART_WORD_LEN_8;
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
        uart_stop_bit = UART_STOP_BIT_1;
        break;

    case STOP_BITS_2:
        uart_stop_bit = UART_STOP_BIT_2;
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
        uart_parity = UART_PARITY_NONE;
        break;

    case PARITY_ODD:
        uart_parity = UART_PARITY_ODD;
        break;

    case PARITY_EVEN:
        uart_parity = UART_PARITY_EVEN;
        break;

    default:
        LOG_E("Unsupported parity.");
        ret = RT_EINVAL;
        goto exit_nu_uart_configure;
    }

    /* Reset this module */
    SYS_ResetModule(psNuUart->rst);

    /* Open Uart and set UART Baudrate */
    UART_Open(base, cfg->baud_rate);

    /* Set line configuration. */
    UART_SetLine_Config(base, 0, uart_word_len, uart_parity, uart_stop_bit);

    /* Enable NVIC interrupt. */
    NVIC_EnableIRQ(psNuUart->irqn);

exit_nu_uart_configure:

    if (ret != RT_EOK)
        UART_Close(base);

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
    UART_T *base = psNuUart->base;

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
    UART_ENABLE_INT(base, UART_INTEN_RLSIEN_Msk | UART_INTEN_RXPDMAEN_Msk);

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
    UART_T *base = psNuUart->base;
    nu_rxbuf_ctx_t psNuRxBufCtx = &psNuUart->dmabuf;

    dma_put_index = nu_pdma_transferred_byte_get(psNuUart->pdma_chanid_rx, psNuRxBufCtx->bufsize);
    if (u32Events & (NU_PDMA_EVENT_TRANSFER_DONE | NU_PDMA_EVENT_TIMEOUT))
    {
        if (u32Events & NU_PDMA_EVENT_TRANSFER_DONE)
        {
            dma_put_index = psNuRxBufCtx->bufsize;
        }
        else if ((u32Events & NU_PDMA_EVENT_TIMEOUT) && !UART_GET_RX_EMPTY(base))
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

    UART_DISABLE_INT(psNuUart->base, UART_INTEN_TXPDMAEN_Msk);// Stop DMA TX transfer

    if (u32Events & NU_PDMA_EVENT_TRANSFER_DONE)
    {
        /* Get base address of uart register */
        UART_T *base = psNuUart->base;

        /* Waiting if TX-FIFO is empty. */
        while (!(UART_IS_TX_EMPTY(base)));

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
    UART_T *base = psNuUart->base;
    if (direction == RT_SERIAL_DMA_TX)
    {
        result = nu_pdma_transfer(psNuUart->pdma_chanid_tx,
                                  8,
                                  (uint32_t)buf,
                                  (uint32_t)base,
                                  size,
                                  0);  // wait-forever
        // Start DMA TX transfer
        UART_ENABLE_INT(base, UART_INTEN_TXPDMAEN_Msk);
    }
    else if (direction == RT_SERIAL_DMA_RX)
    {
        UART_DISABLE_INT(base, UART_INTEN_RLSIEN_Msk | UART_INTEN_RXPDMAEN_Msk | UART_INTEN_TOCNTEN_Msk);

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

    /* Allocate UART_TX nu_dma channel */
    if (psNuUart->pdma_perp_tx != NU_PDMA_UNUSED)
    {
        psNuUart->pdma_chanid_tx = nu_pdma_channel_allocate(psNuUart->pdma_perp_tx);
        if (psNuUart->pdma_chanid_tx >= 0)
        {
            psNuUart->dma_flag |= RT_DEVICE_FLAG_DMA_TX;
        }
    }

    /* Allocate UART_RX nu_dma channel */
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

int8_t nu_uart_get_rx_pdma_chnid(struct rt_serial_device *serial)
{
    nu_uart_t psNuUart = (nu_uart_t)serial;

    RT_ASSERT(serial);

    return psNuUart->pdma_chanid_rx;
}

struct nu_rxbuf_ctx *nu_uart_get_rx_pdma_bufaddr(struct rt_serial_device *serial)
{
    nu_uart_t psNuUart = (nu_uart_t)serial;

    RT_ASSERT(serial);

    return &psNuUart->dmabuf;
}

int8_t nu_uart_get_tx_pdma_chnid(struct rt_serial_device *serial)
{
    nu_uart_t psNuUart = (nu_uart_t)serial;

    RT_ASSERT(serial);

    return psNuUart->pdma_chanid_tx;
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
    UART_T *base = psNuUart->base;

    switch (cmd)
    {
    case RT_DEVICE_CTRL_CLR_INT:
        if (ctrl_arg == RT_DEVICE_FLAG_INT_RX) /* Disable INT-RX */
        {
            UART_DISABLE_INT(base, UART_INTEN_RDAIEN_Msk | UART_INTEN_RXTOIEN_Msk | UART_INTEN_TOCNTEN_Msk);
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
            UART_DISABLE_INT(base, UART_INTEN_RLSIEN_Msk | UART_INTEN_RXPDMAEN_Msk | UART_INTEN_TOCNTEN_Msk);
#endif
        }
        break;

    case RT_DEVICE_CTRL_SET_INT:
        if (ctrl_arg == RT_DEVICE_FLAG_INT_RX) /* Enable INT-RX */
        {
            UART_ENABLE_INT(base, UART_INTEN_RDAIEN_Msk | UART_INTEN_RXTOIEN_Msk | UART_INTEN_TOCNTEN_Msk);
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
        UART_DISABLE_INT(base, UART_INTEN_RLSIEN_Msk | UART_INTEN_RXPDMAEN_Msk);
        UART_DISABLE_INT(base, UART_INTEN_TXPDMAEN_Msk);

        if (psNuUart->dma_flag != 0)
        {
            nu_pdma_channel_terminate(psNuUart->pdma_chanid_tx);
            nu_pdma_channel_terminate(psNuUart->pdma_chanid_rx);
            nu_pdma_uart_rxbuf_free(psNuUart);
        }
#endif

        /* Close UART port */
        UART_Close(base);

        break;

    default:
        /* Flush TX FIFO */
        UART_WAIT_TX_EMPTY(base);
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
    UART_T *base = psNuUart->base;

    /* Waiting if TX-FIFO is full. */
    while (UART_IS_TX_FULL(base));

    /* Put char into TX-FIFO */
    UART_WRITE(base, c);

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
    UART_T *base = psNuUart->base;

    /* Return failure if RX-FIFO is empty. */
    if (UART_GET_RX_EMPTY(base))
    {
        return -1;
    }

    /* Get char from RX-FIFO */
    return UART_READ(base);
}

void nu_uart_set_loopback(struct rt_serial_device *serial, rt_bool_t bOn)
{
    nu_uart_t psNuUart = (nu_uart_t)serial;

    RT_ASSERT(serial);

    /* Get base address of uart register */
    UART_T *base = psNuUart->base;

    bOn ? (base->MODEM |= 0x10) : (base->MODEM &= ~0x10);
}

/**
 * Hardware UART Initialization
 */
rt_err_t rt_hw_uart_init(void)
{
    int i;
    rt_uint32_t flag;
    rt_err_t ret = RT_EOK;

    for (i = (UART_START + 1); i < UART_CNT; i++)
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

    }

    return ret;
}

#endif //#if defined(BSP_USING_UART)
