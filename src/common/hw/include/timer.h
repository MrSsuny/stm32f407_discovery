/*
 * timer.h
 *
 *  Created on: Jul 23, 2025
 *      Author: 2005b
 */

#ifndef SRC_COMMON_HW_INCLUDE_TIMER_H_
#define SRC_COMMON_HW_INCLUDE_TIMER_H_

#include "main.h"
#include "hw_def.h"
#include "stm32f4xx_hal_tim.h"

extern TIM_HandleTypeDef htim2;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */
void timerInit(void);
void MX_TIM2_Init(void);
static inline HAL_StatusTypeDef TIM_IC_Start_IT(TIM_HandleTypeDef *htim, uint32_t Channel)
{
    return HAL_TIM_IC_Start_IT(htim, Channel);
}
void TIM2_Interrupt(void);
void timerEventCheck();

#endif /* SRC_COMMON_HW_INCLUDE_TIMER_H_ */
