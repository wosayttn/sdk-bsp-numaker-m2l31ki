/**************************************************************************//**
*
* @copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
*
* SPDX-License-Identifier: Apache-2.0
*
* Change Logs:
* Date            Author       Notes
* 2024-3-25       Wayne        First version
*
******************************************************************************/

#include <rtconfig.h>

#if defined(BSP_USING_RMC)

#include <rtdevice.h>
#include "NuMicro.h"

#if defined(RT_USING_FAL)
    #include <fal.h>
#endif

/* Private define ---------------------------------------------------------------*/
#define NU_SUPPORT_NONALIGN

#define NU_GETBYTE_OFST(addr)         (((addr)&0x3)*8)
#define NU_GET_WALIGN(addr)           ((addr)&~0x3)
#define NU_GET_LSB2BIT(addr)          ((addr)&0x3)
/* Private typedef --------------------------------------------------------------*/

/* Private functions ------------------------------------------------------------*/
static int nu_rmc_init(void);
#if defined(RT_USING_FAL)
    static int aprom_read(long offset, uint8_t *buf, size_t size);
    static int aprom_write(long offset, const uint8_t *buf, size_t size);
    static int aprom_erase(long offset, size_t size);

    static int ldrom_read(long offset, uint8_t *buf, size_t size);
    static int ldrom_write(long offset, const uint8_t *buf, size_t size);
    static int ldrom_erase(long offset, size_t size);
#endif  /* RT_USING_FAL */

/* Public functions -------------------------------------------------------------*/
int nu_rmc_read(long offset, uint8_t *buf, size_t size);
int nu_rmc_write(long offset, const uint8_t *buf, size_t size);
int nu_rmc_erase(long offset, size_t size);

/* Private variables ------------------------------------------------------------*/
static rt_mutex_t g_mutex_fmc = RT_NULL;

/* Public variables -------------------------------------------------------------*/
#if defined(RT_USING_FAL)
const struct fal_flash_dev OnChip_APROM =
{
    "APROM",
    RMC_APROM_BASE,
    RMC_APROM_END,
    RMC_FLASH_PAGE_SIZE,
    {NULL, aprom_read, aprom_write, aprom_erase}
};

const struct fal_flash_dev OnChip_LDROM =
{
    "LDROM",
    RMC_LDROM_BASE,
    RMC_LDROM_END,
    RMC_FLASH_PAGE_SIZE,
    {NULL, ldrom_read, ldrom_write, ldrom_erase}
};
#endif  /* RT_USING_FAL */

int nu_rmc_read(long addr, uint8_t *buf, size_t size)
{
    size_t read_size = 0;
    uint32_t addr_end = addr + size;
    uint32_t isp_rdata = 0;
    rt_mutex_take(g_mutex_fmc, RT_WAITING_FOREVER);

    uint32_t u32RegLockBackup = SYS_IsRegLocked();

    SYS_UnlockReg();

    if (NU_GET_LSB2BIT(addr))
        isp_rdata = RMC_Read(NU_GET_WALIGN(addr));

    for (; addr < addr_end ;)
    {
        if (NU_GET_LSB2BIT(addr) == 0)
        {
            isp_rdata = RMC_Read(addr);
            if (addr_end - addr >= 4)
            {
                *(uint32_t *)buf = isp_rdata;
                addr += 4;
                buf += 4;
                read_size += 4;
                continue;
            }
        }

        *buf = isp_rdata >> NU_GETBYTE_OFST(addr);
        addr++;
        buf++;
        read_size++;

    }

    if (u32RegLockBackup)
        SYS_LockReg();

    rt_mutex_release(g_mutex_fmc);

    return read_size;
}

int nu_rmc_write(long addr, const uint8_t *buf, size_t size)
{
    size_t write_size = 0;
    uint32_t addr_end = addr + size;
    uint32_t isp_rdata = 0;

    rt_mutex_take(g_mutex_fmc, RT_WAITING_FOREVER);

    uint32_t u32RegLockBackup = SYS_IsRegLocked();
    SYS_UnlockReg();

    if (addr < RMC_APROM_END)
        RMC_ENABLE_AP_UPDATE();
    else if ((addr < RMC_LDROM_END) && addr >= RMC_LDROM_BASE)
        RMC_ENABLE_LD_UPDATE();
    else
    {
        goto Exit2;
    }

    if (NU_GET_LSB2BIT(addr))
        isp_rdata = RMC_Read(NU_GET_WALIGN(addr));

    for (; addr < addr_end ;)
    {

        if (addr_end - addr >= 4 && NU_GET_LSB2BIT(addr) == 0)
        {
            RMC_Write(addr, *((uint32_t *)buf));
            addr += 4;
            buf += 4;
            write_size += 4;
            continue;
        }

        if ((NU_GET_LSB2BIT(addr)) == 0x0)
            isp_rdata = RMC_Read(NU_GET_WALIGN(addr));

        isp_rdata = (isp_rdata & ~(0xFF << NU_GETBYTE_OFST(addr))) | ((*buf) << NU_GETBYTE_OFST(addr));

        if ((NU_GET_LSB2BIT(addr)) == 0x3)
            RMC_Write(NU_GET_WALIGN(addr), isp_rdata);

        addr++;
        buf++;
        write_size++;

    }

    if (NU_GET_LSB2BIT(addr))
        RMC_Write(NU_GET_WALIGN(addr), isp_rdata);

    RMC_DISABLE_AP_UPDATE();
    RMC_DISABLE_LD_UPDATE();

Exit2:

    if (u32RegLockBackup)
        SYS_LockReg();

    rt_mutex_release(g_mutex_fmc);

    return write_size;
}

int nu_rmc_erase(long addr, size_t size)
{
    size_t erased_size = 0;
    uint32_t addrptr;
    uint32_t addr_end = addr + size;
    uint32_t u32RegLockBackup;

#if defined(NU_SUPPORT_NONALIGN)
    uint8_t *page_sdtemp = RT_NULL;
    uint8_t *page_edtemp = RT_NULL;

    addrptr = addr & (RMC_FLASH_PAGE_SIZE - 1);
    if (addrptr)
    {
        page_sdtemp = rt_malloc(addrptr);
        if (page_sdtemp == RT_NULL)
        {
            erased_size = 0;

            goto Exit3;
        }

        if (nu_rmc_read(addr & ~(RMC_FLASH_PAGE_SIZE - 1), page_sdtemp, addrptr) != addrptr)
        {

            erased_size = 0;

            goto Exit3;
        }

    }

    addrptr = addr_end & (RMC_FLASH_PAGE_SIZE - 1);
    if (addrptr)
    {
        page_edtemp = rt_malloc(RMC_FLASH_PAGE_SIZE - addrptr);
        if (page_edtemp == RT_NULL)
        {
            erased_size = 0;

            goto Exit3;
        }

        if (nu_rmc_read(addr_end, page_edtemp, RMC_FLASH_PAGE_SIZE - addrptr) != RMC_FLASH_PAGE_SIZE - addrptr)
        {
            erased_size = 0;

            goto Exit3;
        }

    }
#endif

    rt_mutex_take(g_mutex_fmc, RT_WAITING_FOREVER);

    u32RegLockBackup = SYS_IsRegLocked();

    SYS_UnlockReg();

    if (addr <= RMC_APROM_END)
        RMC_ENABLE_AP_UPDATE();
    else if ((addr < RMC_LDROM_END) && addr >= RMC_LDROM_BASE)
        RMC_ENABLE_LD_UPDATE();
    else
    {
        goto Exit2;
    }

    addrptr = (addr & ~(RMC_FLASH_PAGE_SIZE - 1));
    while (addrptr < addr_end)
    {
        if (RMC_Erase(addrptr) != RT_EOK)
        {
            goto Exit1;
        }
        erased_size += RMC_FLASH_PAGE_SIZE;
        addrptr += RMC_FLASH_PAGE_SIZE;
    }

Exit1:
    RMC_DISABLE_AP_UPDATE();
    RMC_DISABLE_LD_UPDATE();
Exit2:
    if (u32RegLockBackup)
        SYS_LockReg();

    rt_mutex_release(g_mutex_fmc);

#if defined(NU_SUPPORT_NONALIGN)

    if (erased_size >= size)
    {
        addrptr = addr & (RMC_FLASH_PAGE_SIZE - 1);
        if (addrptr)
        {
            if (nu_rmc_write(addr & ~(RMC_FLASH_PAGE_SIZE - 1), page_sdtemp, addrptr) != addrptr)
                goto Exit3;

            erased_size += addrptr;
        }

        addrptr = addr_end & (RMC_FLASH_PAGE_SIZE - 1);
        if (addrptr)
        {

            if (nu_rmc_write(addr_end, page_edtemp, RMC_FLASH_PAGE_SIZE - addrptr) != RMC_FLASH_PAGE_SIZE - addrptr)
                goto Exit3;

            erased_size += RMC_FLASH_PAGE_SIZE - addrptr;

        }
    }
    else
        erased_size = 0;

Exit3:
    if (page_sdtemp != RT_NULL)
        rt_free(page_sdtemp);

    if (page_edtemp != RT_NULL)
        rt_free(page_edtemp);
#endif

    return erased_size;
}

#if defined(RT_USING_FAL)

static int aprom_read(long offset, uint8_t *buf, size_t size)
{
    return nu_rmc_read(OnChip_APROM.addr + offset, buf, size);
}

static int aprom_write(long offset, const uint8_t *buf, size_t size)
{
    return nu_rmc_write(OnChip_APROM.addr + offset, buf, size);
}

static int aprom_erase(long offset, size_t size)
{
    return nu_rmc_erase(OnChip_APROM.addr + offset, size);
}

static int ldrom_read(long offset, uint8_t *buf, size_t size)
{
    return nu_rmc_read(OnChip_LDROM.addr + offset, buf, size);
}

static int ldrom_write(long offset, const uint8_t *buf, size_t size)
{
    return nu_rmc_write(OnChip_LDROM.addr + offset, buf, size);
}

static int ldrom_erase(long offset, size_t size)
{
    return nu_rmc_erase(OnChip_LDROM.addr + offset, size);
}

#endif /* RT_USING_FAL */

static int nu_rmc_init(void)
{
    uint32_t u32RegLockBackup = SYS_IsRegLocked();

    SYS_UnlockReg();
    RMC_ENABLE_ISP();

    rt_kprintf("Executed Bank# is %d.\n", RMC_GET_ISP_BANK_SELECTION());

    if (u32RegLockBackup)
        SYS_LockReg();

    g_mutex_fmc = rt_mutex_create("nu_rmc_lock", RT_IPC_FLAG_PRIO);
    RT_ASSERT(g_mutex_fmc);

#if defined(RT_USING_FAL)
    /* RT_USING_FAL */
    extern int fal_init_check(void);
    if (!fal_init_check())
        fal_init();
#endif

    return (int)RT_EOK;
}
INIT_APP_EXPORT(nu_rmc_init);

#endif /* BSP_USING_RMC */
