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

#ifndef __DRV_PDMA_H__
#define __DRV_PDMA_H__

#include <rtconfig.h>
#include <rtthread.h>
#include "NuMicro.h"

#ifndef NU_PDMA_SGTBL_POOL_SIZE
    #define NU_PDMA_SGTBL_POOL_SIZE (16)
#endif

enum
{
    PDMA_START = -1,
    PDMA0_IDX,
    LPPDMA_IDX,
    PDMA_CNT
};

#define NU_PDMA_CAP_NONE             (0 << 0)

#define NU_PDMA_EVENT_ABORT          (1 << 0)
#define NU_PDMA_EVENT_TRANSFER_DONE  (1 << 1)
#define NU_PDMA_EVENT_ALIGNMENT      (1 << 2)
#define NU_PDMA_EVENT_TIMEOUT        (1 << 3)
#define NU_PDMA_EVENT_ALL            (NU_PDMA_EVENT_ABORT | NU_PDMA_EVENT_TRANSFER_DONE | NU_PDMA_EVENT_TIMEOUT)
#define NU_PDMA_EVENT_MASK           NU_PDMA_EVENT_ALL
#define NU_PDMA_UNUSED               (-1)

#define NU_PDMA_SG_LIMITED_DISTANCE  ((PDMA_DSCT_NEXT_NEXT_Msk>>PDMA_DSCT_NEXT_NEXT_Pos) + 1)
#define NU_PDMA_MAX_TXCNT            ((PDMA_DSCT_CTL_TXCNT_Msk>>PDMA_DSCT_CTL_TXCNT_Pos) + 1)

typedef enum
{
    eMemCtl_SrcFix_DstFix,
    eMemCtl_SrcFix_DstInc,
    eMemCtl_SrcInc_DstFix,
    eMemCtl_SrcInc_DstInc,
    eMemCtl_Undefined = (-1)
} nu_pdma_memctrl_t;

typedef DSCT_T *nu_pdma_desc_t;

typedef void (*nu_pdma_cb_handler_t)(void *, uint32_t);

typedef enum
{
    eCBType_Event,
    eCBType_Trigger,
    eCBType_Disable,
    eCBType_Undefined = (-1)
} nu_pdma_cbtype_t;

typedef struct
{
    union
    {
        uint32_t u32ChID;
        struct
        {
            uint32_t u16ChnIdx : 16;
            uint32_t u16ModIdx : 16;

#define NU_PDMA_CHN_IDX_Pos    (0)
#define NU_PDMA_CHN_IDX_Msk    (0xFFFF<<NU_PDMA_CHN_IDX_Pos)
#define NU_PDMA_IDX_Pos        (16)
#define NU_PDMA_IDX_Msk        (0xFFFF<<NU_PDMA_IDX_Pos)

        };
    };
} pdma_chid_t;

#define NU_PDMA_SET_REQ_SRC_ID(idx, x)    ((idx << NU_PDMA_IDX_Pos) | x)
#define NU_PDMA_GET_REQ_SRC_ID(idx_x)     (idx_x & 0xFFFF)
#define NU_PDMA_GET_CHN_ID                NU_PDMA_GET_REQ_SRC_ID
#define NU_PDMA_GET_IDX(idx_x)            ((idx_x & NU_PDMA_IDX_Msk) >> NU_PDMA_IDX_Pos)

struct nu_pdma_chn_cb
{
    nu_pdma_cbtype_t       m_eCBType;
    nu_pdma_cb_handler_t   m_pfnCBHandler;
    void                  *m_pvUserData;
    uint32_t               m_u32Reserved;
};
typedef struct nu_pdma_chn_cb *nu_pdma_chn_cb_t;

int nu_pdma_channel_allocate(int32_t i32PeripType);
rt_err_t nu_pdma_channel_free(int i32ChannID);
rt_err_t nu_pdma_callback_register(int i32ChannID, nu_pdma_chn_cb_t psChnCb);
rt_err_t nu_pdma_transfer(int i32ChannID, uint32_t u32DataWidth, uint32_t u32AddrSrc, uint32_t u32AddrDst, uint32_t i32TransferCnt, uint32_t u32IdleTimeout_us);
int nu_pdma_transferred_byte_get(int32_t i32ChannID, int32_t i32TriggerByteLen);
void nu_pdma_channel_terminate(int i32ChannID);
nu_pdma_memctrl_t nu_pdma_channel_memctrl_get(int i32ChannID);
rt_err_t nu_pdma_channel_memctrl_set(int i32ChannID, nu_pdma_memctrl_t eMemCtrl);

nu_pdma_cb_handler_t nu_pdma_callback_hijack(int i32ChannID, nu_pdma_cbtype_t eCBType, nu_pdma_chn_cb_t psChnCb_Hijack);
rt_err_t nu_pdma_filtering_set(int i32ChannID, uint32_t u32EventFilter);
uint32_t nu_pdma_filtering_get(int i32ChannID);

// For scatter-gather DMA
rt_err_t nu_pdma_desc_setup(int i32ChannID, nu_pdma_desc_t dma_desc, uint32_t u32DataWidth, uint32_t u32AddrSrc, uint32_t u32AddrDst, int32_t TransferCnt, nu_pdma_desc_t next, uint32_t u32BeSilent);
rt_err_t nu_pdma_sg_transfer(int i32ChannID, nu_pdma_desc_t head, uint32_t u32IdleTimeout_us);
rt_err_t nu_pdma_sgtbls_allocate(int i32ChannID, nu_pdma_desc_t *ppsSgtbls, int num);
void nu_pdma_sgtbls_free(int i32ChannID, nu_pdma_desc_t *ppsSgtbls, int num);

// For memory actor
void *nu_pdma_memcpy(void *dest, void *src, unsigned int count);
rt_size_t nu_pdma_mempush(void *dest, void *src, uint32_t data_width, unsigned int transfer_count);

#endif // __DRV_PDMA_H___
