/*
 * hw_def.h
 *
 *  Created on: Jun 7, 2025
 *      Author: 2005b
 */

#ifndef SRC_HW_HW_DEF_H_
#define SRC_HW_HW_DEF_H_

#include "def.h"
#include "bsp.h"

// -------------------------------------------------------------------
// HW Feature Selection
// -------------------------------------------------------------------
#define _USE_HW_LED
#define      HW_LED_MAX_CH           4

#define _USE_HW_UART
#define      HW_UART_MAX_CH          2

#define _USE_HW_USB
#define _USE_HW_USB_CDC

#define _USE_HW_FLASH

#define _USE_HW_SD

#define _USE_HW_CLI
#define      HW_CLI_CMD_NAME_MAX    16
#define      HW_CLI_CMD_LIST_MAX    16
#define      HW_CLI_LINE_HIS_MAX    4
#define      HW_CLI_LINE_BUF_MAX    32

#define _USE_HW_GPIO
#define      HW_GPIO_MAX_CH         1

#define _PIN_GPIO_SDCARD_DETECT     0
// -------------------------------------------------------------------
// HW Options (DMA, NODMA, etc.)
// -------------------------------------------------------------------

// UART Peripheral DMA/NODMA selection (Required: Select only one)
#define _USE_HW_UART_DMA
//#define _USE_HW_UART_NODMA

#if defined(_USE_HW_UART_DMA) && defined(_USE_HW_UART_NODMA)
#error "UART DMA and NODMA options cannot be enabled simultaneously"
#endif


// DMA/NODMA option for USB CDC <-> UART (Required: Select only one)
#define _USE_HW_USB_CDC_NODMA
// #define _USE_HW_USB_CDC_DMA

#if defined(_USE_HW_USB_CDC_DMA) && defined(_USE_HW_USB_CDC_NODMA)
#error "Only one option (DMA or NODMA) can be selected for USB CDC UART"
#endif



#endif /* SRC_HW_HW_DEF_H_ */
