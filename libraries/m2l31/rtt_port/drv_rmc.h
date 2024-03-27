/**************************************************************************//**
*
* @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*
* SPDX-License-Identifier: Apache-2.0
*
* Change Logs:
* Date            Author           Notes
* 2024-3-25       Wayne            First version
*
******************************************************************************/

#ifndef __DRV_RMC_H__
#define __DRV_RMC_H__

#include <rtthread.h>
#include "NuMicro.h"

int nu_rmc_read(long offset, uint8_t *buf, size_t size);
int nu_rmc_write(long offset, const uint8_t *buf, size_t size);
int nu_rmc_erase(long offset, size_t size);


#endif // __DRV_RMC_H___
