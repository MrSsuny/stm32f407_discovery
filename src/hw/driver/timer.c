/*
 * timer.c
 *
 *  Created on: Jul 23, 2025
 *      Author: 2005b
 */
#include "timer.h"
#include "main.h"
#include "uart.h"
#include "stdbool.h"
#include "stm32f4xx_hal_tim.h"


#define APB1_TIMER_CLK_HZ 84000000UL
#define TICKS_PER_US      (APB1_TIMER_CLK_HZ / 1000000UL)  // = 84

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim6;
extern volatile uint32_t highTime[20];
extern volatile uint32_t  lowTime[20];
extern volatile uint8_t    pulseCount;

/* 캡처 타임스탬프 */
extern volatile uint32_t lastCapTime;

/* 이벤트 구분용 타이머(ms) */
extern volatile uint32_t lastEventTick;
bool firstPulseDetected = false;
void timerInit(void)
{
  MX_TIM2_Init();
  //MX_TIM6_Init();
}
//static inline HAL_StatusTypeDef TIM_IC_Start_IT(TIM_HandleTypeDef *htim, uint32_t Channel)
//{
//    return HAL_TIM_IC_Start_IT(htim, Channel);
//}
/* TIM2 init function */
void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_IC_InitTypeDef sConfigIC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 4294967295;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_IC_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_BOTHEDGE;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* tim_baseHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(tim_baseHandle->Instance==TIM2)
  {
  /* USER CODE BEGIN TIM2_MspInit 0 */

  /* USER CODE END TIM2_MspInit 0 */
    /* TIM2 clock enable */
    __HAL_RCC_TIM2_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**TIM2 GPIO Configuration
    PA0-WKUP     ------> TIM2_CH1
    */
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* TIM2 interrupt Init */
    HAL_NVIC_SetPriority(TIM2_IRQn, 7, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);
  /* USER CODE BEGIN TIM2_MspInit 1 */

  /* USER CODE END TIM2_MspInit 1 */
  }
  else if(tim_baseHandle->Instance==TIM6)
  {
  /* USER CODE BEGIN TIM6_MspInit 0 */

  /* USER CODE END TIM6_MspInit 0 */
    /* TIM6 clock enable */
    __HAL_RCC_TIM6_CLK_ENABLE();

    /* TIM6 interrupt Init */
    HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 7, 0);
    HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
  /* USER CODE BEGIN TIM6_MspInit 1 */

  /* USER CODE END TIM6_MspInit 1 */
  }
}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* tim_baseHandle)
{

  if(tim_baseHandle->Instance==TIM2)
  {
  /* USER CODE BEGIN TIM2_MspDeInit 0 */

  /* USER CODE END TIM2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM2_CLK_DISABLE();

    /**TIM2 GPIO Configuration
    PA0-WKUP     ------> TIM2_CH1
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_1);

    /* TIM2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(TIM2_IRQn);
  /* USER CODE BEGIN TIM2_MspDeInit 1 */

  /* USER CODE END TIM2_MspDeInit 1 */
  }
  else if(tim_baseHandle->Instance==TIM6)
  {
  /* USER CODE BEGIN TIM6_MspDeInit 0 */

  /* USER CODE END TIM6_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM6_CLK_DISABLE();

    /* TIM6 interrupt Deinit */
    HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);
  /* USER CODE BEGIN TIM6_MspDeInit 1 */

  /* USER CODE END TIM6_MspDeInit 1 */
  }
}
void TIM2_Interrupt(void)
{
  //HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET);
  GPIOC->BSRR = GPIO_PIN_7;
  if (TIM2->SR & TIM_SR_CC2IF) {
     uint32_t now = TIM2->CCR2;
     //상승엣지인지 확인
     //GPIO_PinState pin = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1);
     GPIO_PinState pin = (GPIOA->IDR & GPIO_PIN_1) ? GPIO_PIN_SET : GPIO_PIN_RESET;
     //첫번째 pulse 인지 확인
     if(!firstPulseDetected)
     {
       if (pin == GPIO_PIN_SET)
       {
         GPIOC->BSRR = GPIO_PIN_6;
         lastCapTime = now;
         //HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIsN_SET);

       }
       else
       {
         //HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
          GPIOC->BSRR = (uint32_t)GPIO_PIN_6 << 16;   // = (1 << (6 + 16)) = (1 << 22)
         //High 구간 길이 저장
         highTime[pulseCount] = now - lastCapTime;
         lastCapTime = now;

         firstPulseDetected = true;
       }
     }
     else
     {
       if (pin == GPIO_PIN_SET)
       {
         GPIOC->BSRR = GPIO_PIN_6;
         lowTime[pulseCount] = now - lastCapTime;
         lastCapTime = now;
         pulseCount++;
       }
       else //GPIO_PIN_RESET
       {
         GPIOC->BSRR = (uint32_t)GPIO_PIN_6 << 16;   // = (1 << (6 + 16)) = (1 << 22)
         //High 구간 길이 저장
         highTime[pulseCount] = now - lastCapTime;
         lastCapTime = now;

       }
     }
     /* 타임스탬프 업데이트 */
     lastEventTick  = HAL_GetTick();
     //HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);
     GPIOC->BSRR = (uint32_t)GPIO_PIN_7 << 16;   // = (1 << (7 + 16)) = (1 << 23)

    TIM2->SR &= ~TIM_SR_CC2IF; // 인터럽트 플래그 수동 클리어
  }


}
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
//    if (htim->Instance == TIM2 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
//    {
//        uint32_t now = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
//        uint32_t diff_ticks;  // timer ticks 차이
//        uint32_t diff_us;     // 환산된 마이크로초
//        uint32_t diff;
//
//
//
//        //상승엣지인지 확인
//        GPIO_PinState pin = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1);
//
//        //첫번째 pulse 인지 확인
//        if(!firstPulseDetected)
//        {
//          if (pin == GPIO_PIN_SET)
//          {
//            lastCapTime = now;
//            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
//          }
//          else
//          {
//            //High 구간 길이 저장
//            highTime[pulseCount] = now - lastCapTime;
//            lastCapTime = now;
//            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
//            firstPulseDetected = true;
//          }
//        }
//        else
//        {
//          if (pin == GPIO_PIN_SET)
//          {
//            lowTime[pulseCount] = now - lastCapTime;
//            lastCapTime = now;
//            pulseCount++;
//          }
//          else //GPIO_PIN_RESET
//          {
//            //High 구간 길이 저장
//            highTime[pulseCount] = now - lastCapTime;
//            lastCapTime = now;
//          }
//        }
//
//
//
////        /* 타이머 오버플로우 처리 */
////        if (now >= lastCapTime)
////          diff_ticks = now - lastCapTime;
////        else
////          diff_ticks = (0xFFFFFFFF - lastCapTime) + now + 1;
////
////        /* 이벤트 구분: 마지막 이벤트 후 2000 ms 이상 지나면 새 이벤트로 간주 */
////        if ((HAL_GetTick() - lastEventTick) >= 2000)
////        {
////            pulseCount    = 0;
////            lastCapTime   = now;
////        }
//        /* ticks → µs 환산 */
////        diff_us = diff_ticks / TICKS_PER_US;
////        /* 에지 구분: 현재 입력 핀 상태로 판단 */
////        GPIO_PinState pin = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1);
////        //HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_6);
////        if (pin == GPIO_PIN_SET)
////        {
////          HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
////            /* Rising → 이전 Low 종료 */
////            if (pulseCount < 20)
////                lowTime[pulseCount] = diff_us;
////        }
////        else
////        {
////          HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
////            /* Falling → 이전 High 종료 → 한 펄스 완료 */
////            if (pulseCount < 20)
////            {
////                highTime[pulseCount] = diff_us;
////                pulseCount++;
////            }
////        }
//
//        /* 타임스탬프 업데이트 */
//        //lastCapTime    = now;
//        lastEventTick  = HAL_GetTick();
//        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);
//    }
}
void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 8399;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 4999;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
//  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
//  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
//  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
//  {
//    Error_Handler();
//  }
  /* USER CODE BEGIN TIM6_Init 2 */
  __HAL_TIM_ENABLE_IT(&htim6, TIM_IT_UPDATE);   // 켜기
    HAL_TIM_Base_Start_IT(&htim6);
  /* USER CODE END TIM6_Init 2 */

}
void timerEventCheck()
{
/* 이벤트가 진행 중이고, 마지막 캡처 이후 2000ms 경과 → 이벤트 종료 */
    if (pulseCount > 0 && (HAL_GetTick() - lastEventTick) >= 1000)
    {
        char buf[1024];
        int len = 0;

        /* 총 펄스 수 및 각 High/Low 시간 출력 */
        len += snprintf(buf + len, sizeof(buf) - len,
                        "\r\n=== Event End: %d pulses ===\r\n", pulseCount);
        for (uint8_t i = 0; i < pulseCount; i++)
        {
            len += snprintf(buf + len, sizeof(buf) - len,
                            "Pulse %2d: High = %4lu µs, Low = %4lu µs\r\n",
                            i + 1, ((highTime[i] + TICKS_PER_US/2)/TICKS_PER_US), ((lowTime[i] + TICKS_PER_US/2)/TICKS_PER_US));
        }

        uartWrite(_DEF_UART2, (uint8_t *)buf, len);
        //uartPrintf(_DEF_UART2, "Rx : 0x%X\n", rx_data);
        //HAL_UART_Transmit(&huart3, (uint8_t*)buf, len, HAL_MAX_DELAY);

        /* 다음 이벤트를 위해 리셋 */
        pulseCount = 0;
        firstPulseDetected = false;
    }
}
