/*
 * bsp.h
 *
 *  Created on: Jun 7, 2025
 *      Author: 2005b
 */

#ifndef SRC_BSP_BSP_H_
#define SRC_BSP_BSP_H_

#include "def.h"
#include "stm32f4xx_hal.h"
#include "usb_device.h"

void bspInit(void);

void delay(uint32_t ms);
uint32_t millis(void);
void SystemClock_Config(void);
void Error_Handler(void);
void MX_USB_DEVICE_Init(void);
#endif /* SRC_BSP_BSP_H_ */
