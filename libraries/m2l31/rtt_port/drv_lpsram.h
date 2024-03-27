/**************************************************************************//**
*
* @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*
* SPDX-License-Identifier: Apache-2.0
*
* Change Logs:
* Date            Author           Notes
* 2024-01-29      Wayne            First version
*
******************************************************************************/
#include "rtconfig.h"

#if defined(RT_USING_MEMHEAP)

    #include <rthw.h>

    struct rt_memheap *nu_get_lpmemheap(void);

#endif /* #if defined(RT_USING_MEMHEAP) */
