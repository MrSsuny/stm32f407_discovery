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

// -------------------------------------------------------------------
// HW Options (DMA, NODMA, etc.)
// -------------------------------------------------------------------

// UART Peripheral DMA/NODMA selection (Required: Select only one)
// #define _USE_HW_UART_DMA
#define _USE_HW_UART_NODMA

#if defined(_USE_HW_UART_DMA) && defined(_USE_HW_UART_NODMA)
#error "UART DMA and NODMA options cannot be enabled simultaneously"
#endif


// DMA/NODMA option for USB CDC <-> UART (Required: Select only one)
#define _USE_HW_USB_CDC_NODMA
// #define _USE_HW_USB_CDC_DMA

#if defined(_USE_HW_USB_CDC_DMA) && defined(_USE_HW_USB_CDC_NODMA)
#error "Only one option (DMA or NODMA) can be selected for USB CDC UART"
#endif


// #define _USE_HW_UART_PERI

// #if defined(_USE_HW_UART) && defined(_USE_HW_UART_PERI)
// #define _USE_HW_UART_PERI_DMA
// #ifndef _USE_HW_UART_PERI_DMA
// #define _USE_HW_UART_PERI_NODMA
// #endif

// #if defined(_USE_HW_UART_PERI_DMA) && defined(_USE_HW_UART_PERI_NODMA)
// #error "_USE_HW_UART_PERI_DMA and _USE_HW_UART_PERI_NODMA cannot be defined at the same time"
// #endif

// #endif //_USE_HW_UART_PERI


// //USB Peri
// #define _USE_HW_USB_PERI

// #ifdef _USE_HW_USB_PERI

// #define _USE_HW_USB_PERI_CDC
// #if defined(_USE_HW_UART) && defined(_USE_HW_USB_PERI_CDC)

// #define _USE_HW_USB_PERI_CDC_UART_NODMA
// #ifndef _USE_HW_USB_PERI_CDC_UART_NODMA
// #define _USE_HW_USB_PERI_CDC_UART_DMA
// #endif

// #if defined(_USE_HW_UART_PERI_DMA) && defined(_USE_HW_UART_PERI_NODMA)
// #error "_USE_HW_USB_PERI_CDC_UART_DMA and _USE_HW_USB_PERI_CDC_UART_NODMA cannot be defined at the same time"
// #endif

// #endif

// #if defined(_USE_HW_UART) && defined(_USE_HW_UART_PERI)

// #endif

// #endif //_USE_HW_USB_PERI


#endif /* SRC_HW_HW_DEF_H_ */
