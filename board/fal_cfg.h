/**************************************************************************//**
*
* @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*
* SPDX-License-Identifier: Apache-2.0
*
* Change Logs:
* Date            Author       Notes
* 2020-3-03       FYChou       First version
*
******************************************************************************/

#ifndef _FAL_CFG_H_
#define _FAL_CFG_H_

#include <rtconfig.h>
#include "NuMicro.h"

/* ===================== Flash device Configuration ========================= */
#if defined(FAL_PART_HAS_TABLE_CFG)

#if defined(BSP_USING_RMC)
    extern const struct fal_flash_dev OnChip_LDROM;
    extern const struct fal_flash_dev OnChip_APROM;
#endif

#if defined(FAL_USING_SFUD_PORT)
    extern struct fal_flash_dev nor_flash0;
#endif

#if defined(BSP_USING_RMC) && defined(FAL_USING_SFUD_PORT)
#define FAL_FLASH_DEV_TABLE         \
{                                   \
    &OnChip_APROM,                  \
    &OnChip_LDROM,                  \
    &nor_flash0,                    \
}
#define FAL_PART_TABLE                                                        \
{                                                                             \
    {FAL_PART_MAGIC_WORD,   "filesystem",     FAL_USING_NOR_FLASH_DEV_NAME,    0,     (2*1024*1024), 0},  \
    {FAL_PART_MAGIC_WORD,        "loader"     "LDROM",                    0,         RMC_LDROM_SIZE,  0},  \
    {FAL_PART_MAGIC_WORD,          "bank",    "APROM",  RMC_APROM_BANK0_END,          RMC_BANK_SIZE,  0},  \
}

#elif defined(BSP_USING_RMC)

#define FAL_FLASH_DEV_TABLE         \
{                                   \
    &OnChip_APROM,                  \
    &OnChip_LDROM,                  \
}

#define FAL_PART_TABLE                                                        \
{                                                                             \
    {FAL_PART_MAGIC_WORD,        "loader",    "LDROM",                    0, RMC_LDROM_SIZE, 0},  \
    {FAL_PART_MAGIC_WORD,        "bank",      "APROM",  RMC_APROM_BANK0_END,  RMC_BANK_SIZE, 0},  \
}

#elif defined(FAL_USING_SFUD_PORT)

#define FAL_FLASH_DEV_TABLE         \
{                                   \
    &nor_flash0,                    \
}
#define FAL_PART_TABLE                                                        \
{                                                                             \
    {FAL_PART_MAGIC_WORD,   "filesystem",     FAL_USING_NOR_FLASH_DEV_NAME,    0,   2*1024*1024, 0},  \
}

#else

#define FAL_FLASH_DEV_TABLE         \
{                                   \
}

#define FAL_PART_TABLE                                                        \
{                                                                             \
}

#endif

#endif /* FAL_PART_HAS_TABLE_CFG */

#endif /* _FAL_CFG_H_ */
