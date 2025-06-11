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

#define _USE_HW_LED
#define      HW_LED_MAX_CH           4

#define _USE_HW_UART
#define      HW_UART_MAX_CH          2

#define _USE_HW_CDC
//#define _USE_CDC_DMA



#if defined(_USE_HW_CDC) && defined(_USE_CDC_DMA)
    #undef _USE_HW_CDC
#endif


#endif /* SRC_HW_HW_DEF_H_ */
