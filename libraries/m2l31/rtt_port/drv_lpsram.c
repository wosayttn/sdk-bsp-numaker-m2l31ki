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

#include "rtthread.h"

#if defined(RT_USING_MEMHEAP)

#include "NuMicro.h"
#include <rthw.h>

#define LPSRAM_MEMHEAP_NAME     "lpsram"
#define LPSRAM_MEMHEAP_SIZE     (8*1024)
#define LPSRAM_MEMHEAP_BEGIN    (LPSRAM_BASE)
#define LPSRAM_MEMHEAP_END      (LPSRAM_BASE+LPSRAM_MEMHEAP_SIZE-1)

static struct rt_memheap s_lpmemheap;

struct rt_memheap *nu_get_lpmemheap(void)
{
    return &s_lpmemheap;
}

int rt_hw_lpsram_init(void)
{
    return rt_memheap_init(&s_lpmemheap, LPSRAM_MEMHEAP_NAME, (void *)LPSRAM_MEMHEAP_BEGIN, LPSRAM_MEMHEAP_SIZE);
}

#endif /* #if defined(RT_USING_MEMHEAP) */
