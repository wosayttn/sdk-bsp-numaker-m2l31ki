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

#if ((defined(BSP_USING_CRYPTO) || defined(BSP_USING_TRNG) || defined(BSP_USING_CRC)) && defined(RT_USING_HWCRYPTO))

#include <rtdevice.h>
#include <board.h>
#include "NuMicro.h"
#include <nu_bitutil.h>

#if defined(BSP_USING_TRNG)
    #include "drv_trng.h"
#endif

#if defined(BSP_USING_CRC)
    #include "drv_crc.h"
#endif

/* Private typedef --------------------------------------------------------------*/
#define LOG_TAG         "drv.crypto"
#define DBG_ENABLE
#define DBG_SECTION_NAME LOG_TAG
#define DBG_LEVEL         DBG_INFO
#define DBG_COLOR
#include <rtdbg.h>

/* Private functions ------------------------------------------------------------*/
static rt_err_t nu_hwcrypto_create(struct rt_hwcrypto_ctx *ctx);
static void nu_hwcrypto_destroy(struct rt_hwcrypto_ctx *ctx);
static rt_err_t nu_hwcrypto_clone(struct rt_hwcrypto_ctx *des, const struct rt_hwcrypto_ctx *src);
static void nu_hwcrypto_reset(struct rt_hwcrypto_ctx *ctx);

/* Private variables ------------------------------------------------------------*/
static const struct rt_hwcrypto_ops nu_hwcrypto_ops =
{
    .create = nu_hwcrypto_create,
    .destroy = nu_hwcrypto_destroy,
    .copy = nu_hwcrypto_clone,
    .reset = nu_hwcrypto_reset,
};

/* Crypto engine operation ------------------------------------------------------------*/
#if defined(BSP_USING_CRYPTO)

#define NU_HWCRYPTO_AES_NAME    "nu_AES"
#define NU_HWCRYPTO_PRNG_NAME   "nu_PRNG"

static struct rt_mutex s_AES_mutex;

static rt_err_t nu_crypto_init(void)
{
    rt_err_t result = RT_EOK;

    /* init cipher mutex */
#if defined(RT_HWCRYPTO_USING_AES)
    result = rt_mutex_init(&s_AES_mutex, NU_HWCRYPTO_AES_NAME, RT_IPC_FLAG_PRIO);
    RT_ASSERT(result == RT_EOK);
    AES_ENABLE_INT(CRPT);
#endif

    return result;
}

static rt_err_t nu_aes_crypt_run(
    rt_bool_t bEncrypt,
    uint32_t u32OpMode,
    uint8_t *pu8Key,
    uint32_t u32KeySize,
    uint8_t *pu8IV,
    uint8_t *pu8InData,
    uint8_t *pu8OutData,
    uint32_t u32DataLen
)
{
    uint32_t au32SwapKey[8];
    uint32_t au32SwapIV[4];
    rt_err_t result;

    au32SwapKey[0] = nu_get32_be(&pu8Key[0]);
    au32SwapKey[1] = nu_get32_be(&pu8Key[4]);
    au32SwapKey[2] = nu_get32_be(&pu8Key[8]);
    au32SwapKey[3] = nu_get32_be(&pu8Key[12]);

    if ((u32KeySize == AES_KEY_SIZE_192) || (u32KeySize == AES_KEY_SIZE_256))
    {
        au32SwapKey[4] = nu_get32_be(&pu8Key[16]);
        au32SwapKey[5] = nu_get32_be(&pu8Key[20]);
    }

    if (u32KeySize == AES_KEY_SIZE_256)
    {
        au32SwapKey[6] = nu_get32_be(&pu8Key[24]);
        au32SwapKey[7] = nu_get32_be(&pu8Key[28]);
    }

    au32SwapIV[0] = nu_get32_be(&pu8IV[0]);
    au32SwapIV[1] = nu_get32_be(&pu8IV[4]);
    au32SwapIV[2] = nu_get32_be(&pu8IV[8]);
    au32SwapIV[3] = nu_get32_be(&pu8IV[12]);

    result = rt_mutex_take(&s_AES_mutex, RT_WAITING_FOREVER);
    RT_ASSERT(result == RT_EOK);

    //Using Channel 0
    AES_Open(CRPT, 0, bEncrypt, u32OpMode, u32KeySize, AES_IN_OUT_SWAP);
    AES_SetKey(CRPT, 0, (uint32_t *)&au32SwapKey[0], u32KeySize);
    AES_SetInitVect(CRPT, 0, (uint32_t *)au32SwapIV);

    //Setup AES DMA
    AES_SetDMATransfer(CRPT, 0, (uint32_t)pu8InData, (uint32_t)pu8OutData, u32DataLen);
    AES_CLR_INT_FLAG(CRPT);

    /* Start AES encryption/decryption */
    AES_Start(CRPT, 0, CRYPTO_DMA_ONE_SHOT);

    /* Wait done */
    while (!(CRPT->INTSTS & CRPT_INTEN_AESIEN_Msk)) {};

    if ((u32DataLen % 16) && (CRPT->AES_STS & (CRPT_AES_STS_OUTBUFEMPTY_Msk | CRPT_AES_STS_INBUFEMPTY_Msk)))
        rt_kprintf("AES WARNING - AES Data length(%d) is not enough. -> %d \n", u32DataLen, RT_ALIGN(u32DataLen, 16));
    else if (CRPT->INTSTS & (CRPT_INTSTS_AESEIF_Msk) || (CRPT->AES_STS & (CRPT_AES_STS_BUSERR_Msk | CRPT_AES_STS_CNTERR_Msk)))
        rt_kprintf("AES ERROR - CRPT->INTSTS-%08x, CRPT->AES_STS-%08x\n", CRPT->INTSTS, CRPT->AES_STS);

    /* Clear AES interrupt status */
    AES_CLR_INT_FLAG(CRPT);

    result = rt_mutex_release(&s_AES_mutex);
    RT_ASSERT(result == RT_EOK);

    return RT_EOK;
}

static rt_err_t nu_prng_init(void)
{
    uint32_t u32Seed;

#if defined(NU_PRNG_USE_SEED)
    u32Seed = NU_PRNG_SEED_VALUE;
#else
    u32Seed = (uint32_t)rt_tick_get();
#endif

    //Open PRNG 128 bits.
    PRNG_Open(CRPT, PRNG_KEY_SIZE_128, PRNG_SEED_RELOAD, u32Seed);

    return RT_EOK;
}

static rt_uint32_t nu_prng_rand(struct hwcrypto_rng *ctx)
{
    uint32_t au32RNGValue[4];

    PRNG_Start(CRPT);

    PRNG_Read(CRPT, &au32RNGValue[0]);

    return au32RNGValue[0] ^ au32RNGValue[1] ^ au32RNGValue[2] ^ au32RNGValue[3];
}

static rt_err_t nu_aes_crypt(struct hwcrypto_symmetric *symmetric_ctx, struct hwcrypto_symmetric_info *symmetric_info)
{
    uint32_t u32AESOpMode;
    uint32_t u32AESKeySize;
    unsigned char *in, *out;
    unsigned char in_align_flag = 0;
    unsigned char out_align_flag = 0;
    unsigned char iv_temp[16];
    RT_ASSERT(symmetric_ctx != RT_NULL);
    RT_ASSERT(symmetric_info != RT_NULL);

    if ((symmetric_info->length % 4) != 0)
    {
        return -RT_EINVAL;
    }

    //Checking key length
    if (symmetric_ctx->key_bitlen == 128)
    {
        u32AESKeySize = AES_KEY_SIZE_128;
    }
    else if (symmetric_ctx->key_bitlen == 192)
    {
        u32AESKeySize = AES_KEY_SIZE_192;
    }
    else if (symmetric_ctx->key_bitlen == 256)
    {
        u32AESKeySize = AES_KEY_SIZE_256;
    }
    else
    {
        return -RT_EINVAL;
    }

    //Select AES operation mode
    switch (symmetric_ctx->parent.type & (HWCRYPTO_MAIN_TYPE_MASK | HWCRYPTO_SUB_TYPE_MASK))
    {
    case HWCRYPTO_TYPE_AES_ECB:
        u32AESOpMode = AES_MODE_ECB;
        break;
    case HWCRYPTO_TYPE_AES_CBC:
        u32AESOpMode = AES_MODE_CBC;
        break;
    case HWCRYPTO_TYPE_AES_CFB:
        u32AESOpMode = AES_MODE_CFB;
        break;
    case HWCRYPTO_TYPE_AES_OFB:
        u32AESOpMode = AES_MODE_OFB;
        break;
    case HWCRYPTO_TYPE_AES_CTR:
        u32AESOpMode = AES_MODE_CTR;
        break;
    default :
        return -RT_ERROR;
    }

    in = (unsigned char *)symmetric_info->in;
    out = (unsigned char *)symmetric_info->out;

    //Checking in/out data buffer address not alignment or out of SRAM
    if (((rt_uint32_t)in % 4) != 0 || ((rt_uint32_t)in < SRAM_BASE) || ((rt_uint32_t)in > SRAM_END))
    {
        in = rt_malloc(symmetric_info->length);
        if (in == RT_NULL)
        {
            LOG_E("fun[%s] memory allocate %d bytes failed!", __FUNCTION__, symmetric_info->length);
            return -RT_ENOMEM;
        }

        rt_memcpy(in, symmetric_info->in, symmetric_info->length);
        in_align_flag = 1;
    }

    if (((rt_uint32_t)out % 4) != 0 || ((rt_uint32_t)out < SRAM_BASE) || ((rt_uint32_t)out > SRAM_END))
    {
        out = rt_malloc(symmetric_info->length);
        if (out == RT_NULL)
        {
            if (in_align_flag)
                rt_free(in);
            LOG_E("fun[%s] memory allocate %d bytes failed!", __FUNCTION__, symmetric_info->length);
            return -RT_ENOMEM;
        }

        out_align_flag = 1;
    }

    if ((u32AESOpMode == AES_MODE_CBC) && (symmetric_info->mode == HWCRYPTO_MODE_DECRYPT))
    {
        uint32_t loop;

        loop = (symmetric_info->length - 1) / 16;
        rt_memcpy(iv_temp, in + (loop * 16), 16);
    }

    nu_aes_crypt_run(symmetric_info->mode == HWCRYPTO_MODE_ENCRYPT ? TRUE : FALSE, u32AESOpMode, symmetric_ctx->key, u32AESKeySize, symmetric_ctx->iv, in, out, symmetric_info->length);

    if (u32AESOpMode == AES_MODE_CBC)
    {
        if (symmetric_info->mode == HWCRYPTO_MODE_DECRYPT)
        {
            rt_memcpy(symmetric_ctx->iv, iv_temp, 16);
        }
        else
        {
            uint32_t loop;

            loop = (symmetric_info->length - 1) / 16;
            rt_memcpy(symmetric_ctx->iv, out + (loop * 16), 16);
        }
    }

    if (out_align_flag)
    {
        rt_memcpy(symmetric_info->out, out, symmetric_info->length);
        rt_free(out);
    }

    if (in_align_flag)
    {
        rt_free(in);
    }

    return RT_EOK;
}

static const struct hwcrypto_symmetric_ops nu_aes_ops =
{
    .crypt = nu_aes_crypt,
};
#endif

/* CRC operation ------------------------------------------------------------*/
#if defined(BSP_USING_CRC)
static const struct hwcrypto_crc_ops nu_crc_ops =
{
    .update = nu_crc_update,
};
#endif

#if defined(RT_HWCRYPTO_USING_RNG)
    /* RNG operation ------------------------------------------------------------*/
    static struct hwcrypto_rng_ops nu_rng_ops;
#endif

/* Register crypto interface ----------------------------------------------------------*/
static rt_err_t nu_hwcrypto_create(struct rt_hwcrypto_ctx *ctx)
{
    rt_err_t res = RT_EOK;
    RT_ASSERT(ctx != RT_NULL);

    switch (ctx->type & HWCRYPTO_MAIN_TYPE_MASK)
    {
#if defined(RT_HWCRYPTO_USING_RNG)
    case HWCRYPTO_TYPE_RNG:
    {
        ctx->contex = RT_NULL;
        //Setup RNG operation
        ((struct hwcrypto_rng *)ctx)->ops = &nu_rng_ops;
        break;
    }
#endif /* RT_HWCRYPTO_USING_RNG */

#if defined(BSP_USING_CRC) && defined(RT_HWCRYPTO_USING_CRC)
    case HWCRYPTO_TYPE_CRC:
    {
        ctx->contex = RT_NULL;
        //Setup CRC operation
        ((struct hwcrypto_crc *)ctx)->ops = &nu_crc_ops;
        break;
    }
#endif /* BSP_USING_CRC && defined(RT_HWCRYPTO_USING_CRC) */

#if defined(BSP_USING_CRYPTO)
    case HWCRYPTO_TYPE_AES:
    {
        ctx->contex = RT_NULL;
        //Setup AES operation
        ((struct hwcrypto_symmetric *)ctx)->ops = &nu_aes_ops;
        break;
    }
#endif /* BSP_USING_CRYPTO */

    default:
        res = -RT_ERROR;
        break;
    }

    nu_hwcrypto_reset(ctx);

    return res;
}

static void nu_hwcrypto_destroy(struct rt_hwcrypto_ctx *ctx)
{
    RT_ASSERT(ctx != RT_NULL);

    if (ctx->contex)
        rt_free(ctx->contex);
}

static rt_err_t nu_hwcrypto_clone(struct rt_hwcrypto_ctx *des, const struct rt_hwcrypto_ctx *src)
{
    rt_err_t res = RT_EOK;
    RT_ASSERT(des != RT_NULL);
    RT_ASSERT(src != RT_NULL);

    if (des->contex && src->contex)
    {
        rt_memcpy(des->contex, src->contex, sizeof(struct rt_hwcrypto_ctx));
    }
    else
        return -RT_EINVAL;
    return res;
}

static void nu_hwcrypto_reset(struct rt_hwcrypto_ctx *ctx)
{
    switch (ctx->type & HWCRYPTO_MAIN_TYPE_MASK)
    {
    default:
        break;
    }
}

/* Init and register nu_hwcrypto_dev */
int nu_hwcrypto_device_init(void)
{
    rt_err_t result;
    static struct rt_hwcrypto_device nu_hwcrypto_dev;

    nu_hwcrypto_dev.ops = &nu_hwcrypto_ops;
    nu_hwcrypto_dev.id = 0;
    nu_hwcrypto_dev.user_data = &nu_hwcrypto_dev;

#if defined(BSP_USING_CRYPTO)
    nu_crypto_init();
#endif

#if defined(BSP_USING_CRC)
    nu_crc_init();
#endif

#if defined(RT_HWCRYPTO_USING_RNG)
#if defined(BSP_USING_TRNG)
    result = nu_trng_init();
    if (result == RT_EOK)
    {
        LOG_I("TRNG is used as default RNG.");
        nu_rng_ops.update = nu_trng_rand;
    }
    else
#endif
    {
        result = nu_prng_init();
        RT_ASSERT(result == RT_EOK);

        LOG_I("PRNG is used as default RNG.");
        nu_rng_ops.update = nu_prng_rand;
    }
#endif

    /* register hwcrypto operation */
    result = rt_hwcrypto_register(&nu_hwcrypto_dev, RT_HWCRYPTO_DEFAULT_NAME);
    RT_ASSERT(result == RT_EOK);

    return 0;
}
INIT_DEVICE_EXPORT(nu_hwcrypto_device_init);

#endif //#if ((defined(BSP_USING_CRYPTO) || defined(BSP_USING_TRNG) || defined(BSP_USING_CRC)) && defined(RT_USING_HWCRYPTO))
