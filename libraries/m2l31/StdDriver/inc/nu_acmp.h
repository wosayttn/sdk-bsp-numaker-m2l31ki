/**************************************************************************//**
 * @file     nu_acmp.h
 * @version  V1.0
 * @brief    M2L31 series ACMP driver header file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2023 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#ifndef __NU_ACMP_H__
#define __NU_ACMP_H__

#ifdef __cplusplus
extern "C"
{
#endif

/** @addtogroup Standard_Driver Standard Driver
  @{
*/

/** @addtogroup ACMP_Driver ACMP Driver
  @{
*/

/** @addtogroup ACMP_EXPORTED_CONSTANTS ACMP Exported Constants
  @{
*/

/*---------------------------------------------------------------------------------------------------------*/
/* ACMP_CTL constant definitions                                                                           */
/*---------------------------------------------------------------------------------------------------------*/
#define ACMP_CTL_FILTSEL_OFF         (0UL << ACMP_CTL_FILTSEL_Pos) /*!< ACMP_CTL setting for filter function disabled. \hideinitializer */
#define ACMP_CTL_FILTSEL_1PCLK       (1UL << ACMP_CTL_FILTSEL_Pos) /*!< ACMP_CTL setting for 1 PCLK filter count. \hideinitializer */
#define ACMP_CTL_FILTSEL_2PCLK       (2UL << ACMP_CTL_FILTSEL_Pos) /*!< ACMP_CTL setting for 2 PCLK filter count. \hideinitializer */
#define ACMP_CTL_FILTSEL_4PCLK       (3UL << ACMP_CTL_FILTSEL_Pos) /*!< ACMP_CTL setting for 4 PCLK filter count. \hideinitializer */
#define ACMP_CTL_FILTSEL_8PCLK       (4UL << ACMP_CTL_FILTSEL_Pos) /*!< ACMP_CTL setting for 8 PCLK filter count. \hideinitializer */
#define ACMP_CTL_FILTSEL_16PCLK      (5UL << ACMP_CTL_FILTSEL_Pos) /*!< ACMP_CTL setting for 16 PCLK filter count. \hideinitializer */
#define ACMP_CTL_FILTSEL_32PCLK      (6UL << ACMP_CTL_FILTSEL_Pos) /*!< ACMP_CTL setting for 32 PCLK filter count. \hideinitializer */
#define ACMP_CTL_FILTSEL_64PCLK      (7UL << ACMP_CTL_FILTSEL_Pos) /*!< ACMP_CTL setting for 64 PCLK filter count. \hideinitializer */

#define ACMP_CTL_INTPOL_RF           (0UL << ACMP_CTL_INTPOL_Pos)  /*!< ACMP_CTL setting for selecting rising edge and falling edge as interrupt condition. \hideinitializer */
#define ACMP_CTL_INTPOL_R            (1UL << ACMP_CTL_INTPOL_Pos)  /*!< ACMP_CTL setting for selecting rising edge as interrupt condition. \hideinitializer */
#define ACMP_CTL_INTPOL_F            (2UL << ACMP_CTL_INTPOL_Pos)  /*!< ACMP_CTL setting for selecting falling edge as interrupt condition. \hideinitializer */

#define ACMP_CTL_POSSEL_P0           (0UL << ACMP_CTL_POSSEL_Pos)  /*!< ACMP_CTL setting for selecting ACMPx_P0 pin as the source of ACMP V+. \hideinitializer */
#define ACMP_CTL_POSSEL_P1           (1UL << ACMP_CTL_POSSEL_Pos)  /*!< ACMP_CTL setting for selecting ACMPx_P1 pin as the source of ACMP V+. \hideinitializer */
#define ACMP_CTL_POSSEL_P2           (2UL << ACMP_CTL_POSSEL_Pos)  /*!< ACMP_CTL setting for selecting ACMPx_P2 pin as the source of ACMP V+. \hideinitializer */
#define ACMP_CTL_POSSEL_P3           (3UL << ACMP_CTL_POSSEL_Pos)  /*!< ACMP_CTL setting for selecting ACMPx_P3 pin as the source of ACMP V+. \hideinitializer */
#define ACMP_CTL_POSSEL_OPA0         (4UL << ACMP_CTL_POSSEL_Pos)  /*!< ACMP_CTL setting for selecting OPA0 output as the source of ACMP V+. \hideinitializer */
#define ACMP_CTL_POSSEL_OPA1         (5UL << ACMP_CTL_POSSEL_Pos)  /*!< ACMP_CTL setting for selecting OPA0 output as the source of ACMP V+. \hideinitializer */
#define ACMP_CTL_POSSEL_OPA2         (6UL << ACMP_CTL_POSSEL_Pos)  /*!< ACMP_CTL setting for selecting OPA0 output as the source of ACMP V+. \hideinitializer */

#define ACMP_CTL_NEGSEL_PIN          (0UL << ACMP_CTL_NEGSEL_Pos)  /*!< ACMP_CTL setting for selecting the voltage of ACMP negative input pin as the source of ACMP V-. \hideinitializer */
#define ACMP_CTL_NEGSEL_CRV          (1UL << ACMP_CTL_NEGSEL_Pos)  /*!< ACMP_CTL setting for selecting internal comparator reference voltage as the source of ACMP V-. \hideinitializer */
#define ACMP_CTL_NEGSEL_VBG          (2UL << ACMP_CTL_NEGSEL_Pos)  /*!< ACMP_CTL setting for selecting internal Band-gap voltage as the source of ACMP V-. \hideinitializer */
#define ACMP_CTL_NEGSEL_DAC0         (3UL << ACMP_CTL_NEGSEL_Pos)  /*!< ACMP_CTL setting for selecting DAC0 output voltage as the source of ACMP V-. \hideinitializer */
#define ACMP_CTL_NEGSEL_DAC1         (4UL << ACMP_CTL_NEGSEL_Pos)  /*!< ACMP_CTL setting for selecting DAC1 output voltage as the source of ACMP V-. \hideinitializer */

#define ACMP_CTL_HYSTERESIS_40MV     (4UL << ACMP_CTL_HYSSEL_Pos)  /*!< ACMP_CTL setting for enabling the hysteresis function at 40mV. \hideinitializer */
#define ACMP_CTL_HYSTERESIS_20MV     (2UL << ACMP_CTL_HYSSEL_Pos)  /*!< ACMP_CTL setting for enabling the hysteresis function at 20mV. \hideinitializer */
#define ACMP_CTL_HYSTERESIS_DISABLE  (0UL << ACMP_CTL_HYSSEL_Pos)  /*!< ACMP_CTL setting for disabling the hysteresis function. \hideinitializer */

/*---------------------------------------------------------------------------------------------------------*/
/* ACMP_VREF constant definitions                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
#define ACMP_VREF_CRV0SSEL_VDDA      (0UL << ACMP_VREF_CRV0SSEL_Pos)  /*!< ACMP_VREF setting for selecting analog supply voltage VDDA as the CRV0 source voltage \hideinitializer */
#define ACMP_VREF_CRV0SSEL_INTVREF   (1UL << ACMP_VREF_CRV0SSEL_Pos)  /*!< ACMP_VREF setting for selecting internal reference voltage as the CRV0 source voltage \hideinitializer */
#define ACMP_VREF_CRV1SSEL_VDDA      (0UL << ACMP_VREF_CRV1SSEL_Pos)  /*!< ACMP_VREF setting for selecting analog supply voltage VDDA as the CRV1 source voltage \hideinitializer */
#define ACMP_VREF_CRV1SSEL_INTVREF   (1UL << ACMP_VREF_CRV1SSEL_Pos)  /*!< ACMP_VREF setting for selecting internal reference voltage as the CRV1 source voltage \hideinitializer */
#define ACMP_VREF_CRV2SSEL_VDDA      (0UL << ACMP_VREF_CRV2SSEL_Pos)  /*!< ACMP_VREF setting for selecting analog supply voltage VDDA as the CRV2 source voltage \hideinitializer */
#define ACMP_VREF_CRV2SSEL_INTVREF   (1UL << ACMP_VREF_CRV2SSEL_Pos)  /*!< ACMP_VREF setting for selecting internal reference voltage as the CRV2 source voltage \hideinitializer */

/*--------------------------------------------------------------------------------------------------*/
/* Define Error Code                                                                                */
/*--------------------------------------------------------------------------------------------------*/
#define ACMP_TIMEOUT_ERR            (-1)    /*!< ACMP operation abort due to timeout error \hideinitializer */

/*@}*/ /* end of group ACMP_EXPORTED_CONSTANTS */

extern int32_t g_ACMP_i32ErrCode;

/** @addtogroup ACMP_EXPORTED_FUNCTIONS ACMP Exported Functions
  @{
*/

/*---------------------------------------------------------------------------------------------------------*/
/*  Define Macros and functions                                                                            */
/*---------------------------------------------------------------------------------------------------------*/


/**
  * @brief This macro is used to enable output inverse function
  * @param[in] acmp The pointer of the specified ACMP module
  * @param[in] u32ChNum The ACMP number
  * @return None
  * @details This macro will set ACMPOINV bit of ACMP_CTL register to enable output inverse function.
  * \hideinitializer
  */
#define ACMP_ENABLE_OUTPUT_INVERSE(acmp, u32ChNum) \
    if (acmp == ACMP01) {(acmp)->CTL[u32ChNum] |= ACMP_CTL_ACMPOINV_Msk;}\
    else if (acmp == ACMP2) {(acmp)->CTL[0] |= ACMP_CTL_ACMPOINV_Msk;}

/**
  * @brief This macro is used to disable output inverse function
  * @param[in] acmp The pointer of the specified ACMP module
  * @param[in] u32ChNum The ACMP number
  * @return None
  * @details This macro will clear ACMPOINV bit of ACMP_CTL register to disable output inverse function.
  * \hideinitializer
  */
#define ACMP_DISABLE_OUTPUT_INVERSE(acmp, u32ChNum) \
    if (acmp == ACMP01) {(acmp)->CTL[u32ChNum] &= ~ACMP_CTL_ACMPOINV_Msk);}\
    else if (acmp == ACMP2) {(acmp)->CTL[0] &= ~ACMP_CTL_ACMPOINV_Msk);}

/**
  * @brief This macro is used to select ACMP negative input source
  * @param[in] acmp The pointer of the specified ACMP module
  * @param[in] u32ChNum The ACMP number
  * @param[in] u32Src is comparator negative input selection. Including:
  *                  - \ref ACMP_CTL_NEGSEL_PIN
  *                  - \ref ACMP_CTL_NEGSEL_CRV
  *                  - \ref ACMP_CTL_NEGSEL_VBG
  *                  - \ref ACMP_CTL_NEGSEL_DAC0
  *                  - \ref ACMP_CTL_NEGSEL_DAC1
  * @return None
  * @details This macro will set NEGSEL to determine the source of negative input.
  * \hideinitializer
  */
#define ACMP_SET_NEG_SRC(acmp, u32ChNum, u32Src) \
    if (acmp == ACMP01) {(acmp)->CTL[u32ChNum] = ((acmp)->CTL[u32ChNum] & ~ACMP_CTL_NEGSEL_Msk) | (u32Src);}\
    else if (acmp == ACMP2) {(acmp)->CTL[0] = ((acmp)->CTL[0] & ~ACMP_CTL_NEGSEL_Msk) | (u32Src);}

/**
  * @brief This macro is used to enable hysteresis function and set hysteresis to 20mV
  * @param[in] acmp The pointer of the specified ACMP module
  * @param[in] u32ChNum The ACMP number
  * @return None
  * \hideinitializer
  */
#define ACMP_ENABLE_HYSTERESIS(acmp, u32ChNum) \
    if (acmp == ACMP01) {(acmp)->CTL[u32ChNum] = ((acmp)->CTL[u32ChNum] & ~ACMP_CTL_HYSSEL_Msk) | (ACMP_CTL_HYSTERESIS_20MV);}\
    else if (acmp == ACMP2) {(acmp)->CTL[0] = ((acmp)->CTL[0] & ~ACMP_CTL_HYSSEL_Msk) | (ACMP_CTL_HYSTERESIS_20MV);}

/**
  * @brief This macro is used to disable hysteresis function
  * @param[in] acmp The pointer of the specified ACMP module
  * @param[in] u32ChNum The ACMP number
  * @return None
  * @details This macro will clear HYSEL bits of ACMP_CTL register to disable hysteresis function.
  * \hideinitializer
  */
#define ACMP_DISABLE_HYSTERESIS(acmp, u32ChNum) \
    if (acmp == ACMP01) {(acmp)->CTL[u32ChNum] = ((acmp)->CTL[u32ChNum] & ~ACMP_CTL_HYSSEL_Msk) | (ACMP_CTL_HYSTERESIS_DISABLE);}\
    else if (acmp == ACMP2) {(acmp)->CTL[0] = ((acmp)->CTL[0] & ~ACMP_CTL_HYSSEL_Msk) | (ACMP_CTL_HYSTERESIS_DISABLE);}

/**
  * @brief This macro is used to select hysteresis level
  * @param[in] acmp The pointer of the specified ACMP module
  * @param[in] u32ChNum The ACMP number
  * @param[in] u32HysSel The hysteresis function option. Including:
  *                  - \ref ACMP_CTL_HYSTERESIS_40MV
  *                  - \ref ACMP_CTL_HYSTERESIS_20MV
  *                  - \ref ACMP_CTL_HYSTERESIS_DISABLE
  * \hideinitializer
  * @return None
  */
#define ACMP_CONFIG_HYSTERESIS(acmp, u32ChNum, u32HysSel) \
    if (acmp == ACMP01) {(acmp)->CTL[u32ChNum] = ((acmp)->CTL[u32ChNum] & ~ACMP_CTL_HYSSEL_Msk) | (u32HysSel);}\
    else if (acmp == ACMP2) {(acmp)->CTL[0] = ((acmp)->CTL[0] & ~ACMP_CTL_HYSSEL_Msk) | (u32HysSel);}

/**
  * @brief This macro is used to enable interrupt
  * @param[in] acmp The pointer of the specified ACMP module
  * @param[in] u32ChNum The ACMP number
  * @return None
  * @details This macro will set ACMPIE bit of ACMP_CTL register to enable interrupt function.
  *          If wake-up function is enabled, the wake-up interrupt will be enabled as well.
  * \hideinitializer
  */
#define ACMP_ENABLE_INT(acmp, u32ChNum) \
    if (acmp == ACMP01) {(acmp)->CTL[u32ChNum] |= ACMP_CTL_ACMPIE_Msk;}\
    else if (acmp == ACMP2) {(acmp)->CTL[0] |= ACMP_CTL_ACMPIE_Msk;}

/**
  * @brief This macro is used to disable interrupt
  * @param[in] acmp The pointer of the specified ACMP module
  * @param[in] u32ChNum The ACMP number
  * @return None
  * @details This macro will clear ACMPIE bit of ACMP_CTL register to disable interrupt function.
  * \hideinitializer
  */
#define ACMP_DISABLE_INT(acmp, u32ChNum) \
    if (acmp == ACMP01) {(acmp)->CTL[u32ChNum] &= ~ACMP_CTL_ACMPIE_Msk;}\
    else if (acmp == ACMP2) {(acmp)->CTL[0] &= ~ACMP_CTL_ACMPIE_Msk;}

/**
  * @brief This macro is used to enable ACMP
  * @param[in] acmp The pointer of the specified ACMP module
  * @param[in] u32ChNum The ACMP number
  * @return None
  * @details This macro will set ACMPEN bit of ACMP_CTL register to enable analog comparator.
  * \hideinitializer
  */
#define ACMP_ENABLE(acmp, u32ChNum) \
    if (acmp == ACMP01) {(acmp)->CTL[u32ChNum] |= ACMP_CTL_ACMPEN_Msk;}\
    else if (acmp == ACMP2) {(acmp)->CTL[0] |= ACMP_CTL_ACMPEN_Msk;}

/**
  * @brief This macro is used to disable ACMP
  * @param[in] acmp The pointer of the specified ACMP module
  * @param[in] u32ChNum The ACMP number
  * @return None
  * @details This macro will clear ACMPEN bit of ACMP_CTL register to disable analog comparator.
  * \hideinitializer
  */
#define ACMP_DISABLE(acmp, u32ChNum) \
    if (acmp == ACMP01) {(acmp)->CTL[u32ChNum] &= ~ACMP_CTL_ACMPEN_Msk;}\
    else if (acmp == ACMP2) {(acmp)->CTL[0] &= ~ACMP_CTL_ACMPEN_Msk;}

/**
  * @brief This macro is used to get ACMP output value
  * @param[in] acmp The pointer of the specified ACMP module
  * @param[in] u32ChNum The ACMP number
  * @return  ACMP output value
  * @details This macro will return the ACMP output value.
  * \hideinitializer
  */
#define ACMP_GET_OUTPUT(acmp, u32ChNum) \
    ( (acmp == ACMP01) ? \
      (((acmp)->STATUS & (ACMP_STATUS_ACMPO0_Msk<<(u32ChNum)))?1:0) : \
      (((acmp)->STATUS & (ACMP_STATUS_ACMPO0_Msk<<(0)))?1:0) )

/**
  * @brief This macro is used to get ACMP interrupt flag
  * @param[in] acmp The pointer of the specified ACMP module
  * @param[in] u32ChNum The ACMP number
  * @return   ACMP interrupt occurred (1) or not (0)
  * @details This macro will return the ACMP interrupt flag.
  * \hideinitializer
  */
#define ACMP_GET_INT_FLAG(acmp, u32ChNum) \
    ( (acmp == ACMP01) ? \
      (((acmp)->STATUS & (ACMP_STATUS_ACMPIF0_Msk<<(u32ChNum)))?1:0) : \
      (((acmp)->STATUS & (ACMP_STATUS_ACMPIF0_Msk<<(0)))?1:0) )

/**
  * @brief This macro is used to clear ACMP interrupt flag
  * @param[in] acmp The pointer of the specified ACMP module
  * @param[in] u32ChNum The ACMP number
  * @return   None
  * @details This macro will write 1 to ACMPIFn bit of ACMP_STATUS register to clear interrupt flag.
  * \hideinitializer
  */
#define ACMP_CLR_INT_FLAG(acmp, u32ChNum) \
    if (acmp == ACMP01) {(acmp)->STATUS = (ACMP_STATUS_ACMPIF0_Msk<<(u32ChNum));}\
    else if (acmp == ACMP2) {(acmp)->STATUS = (ACMP_STATUS_ACMPIF0_Msk<<(0));}

/**
  * @brief This macro is used to clear ACMP wake-up interrupt flag
  * @param[in] acmp The pointer of the specified ACMP module
  * @param[in] u32ChNum The ACMP number
  * @return   None
  * @details This macro will write 1 to WKIFn bit of ACMP_STATUS register to clear interrupt flag.
  * \hideinitializer
  */
#define ACMP_CLR_WAKEUP_INT_FLAG(acmp, u32ChNum) \
    if (acmp == ACMP01) {(acmp)->STATUS = (ACMP_STATUS_WKIF0_Msk<<(u32ChNum));}\
    else if (acmp == ACMP2) {(acmp)->STATUS = (ACMP_STATUS_WKIF0_Msk<<(0));}

/**
  * @brief This macro is used to enable ACMP wake-up function
  * @param[in] acmp The pointer of the specified ACMP module
  * @param[in] u32ChNum The ACMP number
  * @return None
  * @details This macro will set WKEN to enable ACMP wake-up function.
  * \hideinitializer
  */
#define ACMP_ENABLE_WAKEUP(acmp, u32ChNum) \
    if (acmp == ACMP01) {(acmp)->CTL[u32ChNum] |= ACMP_CTL_WKEN_Msk;}\
    else if (acmp == ACMP2) {(acmp)->CTL[0] |= ACMP_CTL_WKEN_Msk;}

/**
  * @brief This macro is used to disable ACMP wake-up function
  * @param[in] acmp The pointer of the specified ACMP module
  * @param[in] u32ChNum The ACMP number
  * @return None
  * @details This macro will clear WKEN to disable ACMP wake-up function.
  * \hideinitializer
  */
#define ACMP_DISABLE_WAKEUP(acmp, u32ChNum) \
    if (acmp == ACMP01) {(acmp)->CTL[u32ChNum] &= ~ACMP_CTL_WKEN_Msk;}\
    else if (acmp == ACMP2) {(acmp)->CTL[0] &= ~ACMP_CTL_WKEN_Msk;}

/**
  * @brief This macro is used to select ACMP positive input pin
  * @param[in] acmp The pointer of the specified ACMP module
  * @param[in] u32ChNum The ACMP number
  * @param[in] u32Pin Comparator positive pin selection. Including:
  *                  - \ref ACMP_CTL_POSSEL_P0
  *                  - \ref ACMP_CTL_POSSEL_P1
  *                  - \ref ACMP_CTL_POSSEL_P2
  *                  - \ref ACMP_CTL_POSSEL_P3
  *                  - \ref ACMP_CTL_POSSEL_OPA0
  *                  - \ref ACMP_CTL_POSSEL_OPA1
  *                  - \ref ACMP_CTL_POSSEL_OPA2
  * @return None
  * @details This macro will set POSSEL to determine the comparator positive input pin.
  * \hideinitializer
  */
#define ACMP_SELECT_P(acmp, u32ChNum, u32Pin) \
    if (acmp == ACMP01) {(acmp)->CTL[u32ChNum] = ((acmp)->CTL[u32ChNum] & ~ACMP_CTL_POSSEL_Msk) | (u32Pin);}\
    else if (acmp == ACMP2) {(acmp)->CTL[0] = ((acmp)->CTL[0] & ~ACMP_CTL_POSSEL_Msk) | (u32Pin);}

/**
  * @brief This macro is used to enable ACMP filter function
  * @param[in] acmp The pointer of the specified ACMP module
  * @param[in] u32ChNum The ACMP number
  * @return None
  * @details This macro will set OUTSEL to enable output filter function.
  * \hideinitializer
  */
#define ACMP_ENABLE_FILTER(acmp, u32ChNum) \
    if (acmp == ACMP01) {(acmp)->CTL[u32ChNum] |= ACMP_CTL_OUTSEL_Msk;}\
    else if (acmp == ACMP2) {(acmp)->CTL[0] |= ACMP_CTL_OUTSEL_Msk;}

/**
  * @brief This macro is used to disable ACMP filter function
  * @param[in] acmp The pointer of the specified ACMP module
  * @param[in] u32ChNum The ACMP number
  * @return None
  * @details This macro will clear OUTSEL to disable output filter function.
  * \hideinitializer
  */
#define ACMP_DISABLE_FILTER(acmp, u32ChNum) \
    if (acmp == ACMP01) {(acmp)->CTL[u32ChNum] &= ~ACMP_CTL_OUTSEL_Msk;}\
    else if (acmp == ACMP2) {(acmp)->CTL[0] &= ~ACMP_CTL_OUTSEL_Msk;}

/**
  * @brief This macro is used to set ACMP filter function
  * @param[in] acmp The pointer of the specified ACMP module
  * @param[in] u32ChNum The ACMP number
  * @param[in] u32Cnt is comparator filter count setting.
  *                  - \ref ACMP_CTL_FILTSEL_OFF
  *                  - \ref ACMP_CTL_FILTSEL_1PCLK
  *                  - \ref ACMP_CTL_FILTSEL_2PCLK
  *                  - \ref ACMP_CTL_FILTSEL_4PCLK
  *                  - \ref ACMP_CTL_FILTSEL_8PCLK
  *                  - \ref ACMP_CTL_FILTSEL_16PCLK
  *                  - \ref ACMP_CTL_FILTSEL_32PCLK
  *                  - \ref ACMP_CTL_FILTSEL_64PCLK
  * @return None
  * @details When ACMP output filter function is enabled, the output sampling count is determined by FILTSEL.
  * \hideinitializer
  */
#define ACMP_SET_FILTER(acmp, u32ChNum, u32Cnt) \
    if (acmp == ACMP01) {(acmp)->CTL[u32ChNum] = ((acmp)->CTL[u32ChNum] & ~ACMP_CTL_FILTSEL_Msk) | (u32Cnt);}\
    else if (acmp == ACMP2) {(acmp)->CTL[0] = ((acmp)->CTL[0] & ~ACMP_CTL_FILTSEL_Msk) | (u32Cnt);}

/**
  * @brief This macro is used to select comparator reference voltage CRV0
  * @param[in] acmp The pointer of the specified ACMP module
  * @param[in] u32Level  The comparator reference voltage setting.
  *             The formula is:
  *                       comparator reference voltage CRV0 = CRV0 source voltage * u32Level / 63
  *             The range of u32Level is 0 ~ 63.
  * @return   None
  * @details  When CRV0 is selected as ACMP negative input source, the CRV0 level is determined by CRV0SEL.
  * \hideinitializer
  */
#define ACMP_CRV0_SEL(acmp, u32Level) ((acmp)->VREF = ((acmp)->VREF & ~ACMP_VREF_CRV0SEL_Msk) | ((u32Level)<<ACMP_VREF_CRV0SEL_Pos))

/**
  * @brief This macro is used to select the source of CRV0
  * @param[in] acmp The pointer of the specified ACMP module
  * @param[in] u32Src is the source of CRV0. Including:
  *                  - \ref ACMP_VREF_CRV0SSEL_VDDA
  *                  - \ref ACMP_VREF_CRV0SSEL_INTVREF
  * @return None
  * @details The source of CRV0 can be VDDA or internal reference voltage. The internal reference voltage level is determined by SYS_VREFCTL register.
  * \hideinitializer
  */
#define ACMP_SELECT_CRV0_SRC(acmp, u32Src) ((acmp)->VREF = ((acmp)->VREF & ~ACMP_VREF_CRV0SSEL_Msk) | (u32Src))

/**
  * @brief This macro is used to select comparator reference voltage CRV1
  * @param[in] acmp The pointer of the specified ACMP module
  * @param[in] u32Level  The comparator reference voltage setting.
  *             The formula is:
  *                       comparator reference voltage CRV1 = CRV1 source voltage * u32Level / 63
  *             The range of u32Level is 0 ~ 63.
  * @return   None
  * @details  When CRV1 is selected as ACMP negative input source, the CRV1 level is determined by CRV1SEL.
  * \hideinitializer
  */
#define ACMP_CRV1_SEL(acmp, u32Level) ((acmp)->VREF = ((acmp)->VREF & ~ACMP_VREF_CRV1SEL_Msk) | ((u32Level)<<ACMP_VREF_CRV1SEL_Pos))

/**
  * @brief This macro is used to select the source of CRV1
  * @param[in] acmp The pointer of the specified ACMP module
  * @param[in] u32Src is the source of CRV1. Including:
  *                  - \ref ACMP_VREF_CRV1SSEL_VDDA
  *                  - \ref ACMP_VREF_CRV1SSEL_INTVREF
  * @return None
  * @details The source of CRV1 can be VDDA or internal reference voltage. The internal reference voltage level is determined by SYS_VREFCTL register.
  * \hideinitializer
  */
#define ACMP_SELECT_CRV1_SRC(acmp, u32Src) ((acmp)->VREF = ((acmp)->VREF & ~ACMP_VREF_CRV1SSEL_Msk) | (u32Src))

/**
  * @brief This macro is used to select comparator reference voltage CRV2
  * @param[in] acmp The pointer of the specified ACMP module
  * @param[in] u32Level  The comparator reference voltage setting.
  *             The formula is:
  *                       comparator reference voltage CRV2 = CRV2 source voltage * u32Level / 63
  *             The range of u32Level is 0 ~ 63.
  * @return   None
  * @details  When CRV2 is selected as ACMP negative input source, the CRV2 level is determined by CRV2SEL.
  * \hideinitializer
  */
#define ACMP_CRV2_SEL(acmp, u32Level) ((acmp)->VREF = ((acmp)->VREF & ~ACMP_VREF_CRV2SEL_Msk) | ((u32Level)<<ACMP_VREF_CRV2SEL_Pos))

/**
  * @brief This macro is used to select the source of CRV2
  * @param[in] acmp The pointer of the specified ACMP module
  * @param[in] u32Src is the source of CRV2. Including:
  *                  - \ref ACMP_VREF_CRV2SSEL_VDDA
  *                  - \ref ACMP_VREF_CRV2SSEL_INTVREF
  * @return None
  * @details The source of CRV2 can be VDDA or internal reference voltage. The internal reference voltage level is determined by SYS_VREFCTL register.
  * \hideinitializer
  */
#define ACMP_SELECT_CRV2_SRC(acmp, u32Src) ((acmp)->VREF = ((acmp)->VREF & ~ACMP_VREF_CRV2SSEL_Msk) | (u32Src))

/**
  * @brief This macro is used to select ACMP interrupt condition
  * @param[in] acmp The pointer of the specified ACMP module
  * @param[in] u32ChNum The ACMP number
  * @param[in] u32Cond Comparator interrupt condition selection. Including:
  *                  - \ref ACMP_CTL_INTPOL_RF
  *                  - \ref ACMP_CTL_INTPOL_R
  *                  - \ref ACMP_CTL_INTPOL_F
  * @return None
  * @details The ACMP output interrupt condition can be rising edge, falling edge or any edge.
  * \hideinitializer
  */
#define ACMP_SELECT_INT_COND(acmp, u32ChNum, u32Cond) \
    if (acmp == ACMP01) {(acmp)->CTL[u32ChNum] = ((acmp)->CTL[u32ChNum] & ~ACMP_CTL_INTPOL_Msk) | (u32Cond);}\
    else if (acmp == ACMP2) {(acmp)->CTL[0] = ((acmp)->CTL[0] & ~ACMP_CTL_INTPOL_Msk) | (u32Cond);}

/**
  * @brief This macro is used to enable ACMP window latch mode
  * @param[in] acmp The pointer of the specified ACMP module
  * @param[in] u32ChNum The ACMP number
  * @return None
  * @details This macro will set WLATEN to enable ACMP window latch mode.
  *          When ACMP_WLAT pin is at high level, ACMPO passes through window latch
  *          block; when ACMP_WLAT pin is at low level, the output of window latch block,
  *          WLATOUT, is frozen.
  * \hideinitializer
  */
#define ACMP_ENABLE_WINDOW_LATCH(acmp, u32ChNum) \
    if (acmp == ACMP01) {(acmp)->CTL[u32ChNum] |= ACMP_CTL_WLATEN_Msk;}\
    else if (acmp == ACMP2) {(acmp)->CTL[0] |= ACMP_CTL_WLATEN_Msk;}

/**
  * @brief This macro is used to disable ACMP window latch mode
  * @param[in] acmp The pointer of the specified ACMP module
  * @param[in] u32ChNum The ACMP number
  * @return None
  * @details This macro will clear WLATEN to disable ACMP window latch mode.
  * \hideinitializer
  */
#define ACMP_DISABLE_WINDOW_LATCH(acmp, u32ChNum) \
    if (acmp == ACMP01) {(acmp)->CTL[u32ChNum] &= ~ACMP_CTL_WLATEN_Msk;}\
    else if (acmp == ACMP2) {(acmp)->CTL[0] &= ~ACMP_CTL_WLATEN_Msk;}

/**
  * @brief This macro is used to enable ACMP window compare mode
  * @param[in] acmp The pointer of the specified ACMP module
  * @param[in] u32ChNum The ACMP number
  * @return None
  * @details This macro will set WCMPSEL to enable ACMP window compare mode.
  *          When window compare mode is enabled, user can connect the specific analog voltage
  *          source to either the positive inputs of both comparators or the negative inputs of
  *          both comparators. The upper bound and lower bound of the designated range are
  *          determined by the voltages applied to the other inputs of both comparators. If the
  *          output of a comparator is low and the other comparator outputs high, which means two
  *          comparators implies the upper and lower bound. User can directly monitor a specific
  *          analog voltage source via ACMPWO.
  * \hideinitializer
  */
#define ACMP_ENABLE_WINDOW_COMPARE(acmp, u32ChNum) ((acmp)->CTL[u32ChNum] |= ACMP_CTL_WCMPSEL_Msk)

/**
  * @brief This macro is used to disable ACMP window compare mode
  * @param[in] acmp The pointer of the specified ACMP module
  * @param[in] u32ChNum The ACMP number
  * @return None
  * @details This macro will clear WCMPSEL to disable ACMP window compare mode.
  * \hideinitializer
  */
#define ACMP_DISABLE_WINDOW_COMPARE(acmp, u32ChNum) ((acmp)->CTL[u32ChNum] &= ~ACMP_CTL_WCMPSEL_Msk)

/* Function prototype declaration */
void ACMP_Open(ACMP_T *acmp, uint32_t u32ChNum, uint32_t u32NegSrc, uint32_t u32HysSel);
void ACMP_Close(ACMP_T *acmp, uint32_t u32ChNum);

/*@}*/ /* end of group ACMP_EXPORTED_FUNCTIONS */

/*@}*/ /* end of group ACMP_Driver */

/*@}*/ /* end of group Standard_Driver */

#ifdef __cplusplus
}
#endif

#endif /* __NU_ACMP_H__ */

/*** (C) COPYRIGHT 2023 Nuvoton Technology Corp. ***/
