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

#ifndef __DRV_UART_H__
#define __DRV_UART_H__

#include <rtdevice.h>

rt_err_t rt_hw_uart_init(void);
void nu_uart_set_rs485aud(struct rt_serial_device *serial, rt_bool_t bRTSActiveLowLevel);

#endif /* __DRV_UART_H__ */
