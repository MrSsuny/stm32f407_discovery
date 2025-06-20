/*
 * uart.c
 *
 *  Created on: Jun 11, 2025
 *      Author: 2005b
 */

#include "uart.h"
#include "cdc.h"
#include "qbuffer.h"


//여기서만 사용되는 변수는 static으로 하는게 좋다.
static bool is_open[UART_MAX_CH];

static qbuffer_t qbuffer[UART_MAX_CH];
static uint8_t rx_buf[256];

#ifdef _USE_HW_UART_NODMA
static uint8_t rx_data[UART_MAX_CH]; //IT 모드일때 사용
#endif
//UART_HandleTypeDef huart1;
UART_HandleTypeDef huart3; //PB10,11
//DMA_HandleTypeDef hdma_usart1_rx;
#ifdef _USE_HW_UART_DMA
DMA_HandleTypeDef hdma_usart3_rx;
#endif

bool uartInit(void)
{
  for(int i=0;i<UART_MAX_CH;i++)
  {
    is_open[i] = false;
  }

  return true;
}
bool uartOpen(uint8_t ch, uint32_t baud)
{
  bool ret = false;
  switch(ch)
  {
    case _DEF_UART1: //USB CDC
      is_open[ch] = true;
      ret = true;
      break;
    case _DEF_UART2:
      //PB10,11
        huart3.Instance = USART3;
        huart3.Init.BaudRate = 115200;
        huart3.Init.WordLength = UART_WORDLENGTH_8B;
        huart3.Init.StopBits = UART_STOPBITS_1;
        huart3.Init.Parity = UART_PARITY_NONE;
        huart3.Init.Mode = UART_MODE_TX_RX;
        huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
        huart3.Init.OverSampling = UART_OVERSAMPLING_16;
        if (HAL_UART_Init(&huart3) != HAL_OK)
        {
          Error_Handler();
        }
       //
//      huart1.Instance = USART1;
//      huart1.Init.BaudRate = baud;
//      huart1.Init.WordLength = UART_WORDLENGTH_8B;
//      huart1.Init.StopBits = UART_STOPBITS_1;
//      huart1.Init.Parity = UART_PARITY_NONE;
//      huart1.Init.Mode = UART_MODE_TX_RX;
//      huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
//      huart1.Init.OverSampling = UART_OVERSAMPLING_16;

        //기존에 open 되어있었는데 다시 속도를 바꿔서 오픈하는 경우
//      HAL_UART_DeInit(&huart1);
        HAL_UART_DeInit(&huart3);
//
//
        qbufferCreate(&qbuffer[ch], rx_buf,256);
//
////DMA 설정
//      /* DMA controller clock enable */
//      __HAL_RCC_DMA2_CLK_ENABLE();
//      /* DMA interrupt init */
//      /* DMA2_Stream2_IRQn interrupt configuration */
//      HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, 0, 0);
//      HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);
////DMA 설정 여기까지
//
#ifdef _USE_HW_UART_DMA
      /* DMA controller clock enable */
      __HAL_RCC_DMA1_CLK_ENABLE();

      /* DMA interrupt init */
      /* DMA1_Stream1_IRQn interrupt configuration */
      HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 0, 0);
      HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);
#endif


//    if (HAL_UART_Init(&huart1) != HAL_OK)
      if (HAL_UART_Init(&huart3) != HAL_OK)
      {
        ret = false;
        is_open[ch] = true;
      }
      else
      {
        ret = true;
        is_open[ch] = true;
//
//        //DMA 일때 이걸로
//        if(HAL_UART_Receive_DMA(&huart1, (uint8_t *)&rx_buf[0],256) != HAL_OK)
//        {
//          ret = false;
//        }
//        qbuffer[ch].in = qbuffer[ch].len - hdma_usart1_rx.Instance->NDTR;
//        qbuffer[ch].out = qbuffer[ch].in; //buffer는 flush 된다.(ring buffer 의미상)
#ifdef _USE_HW_UART_DMA
//https://www.youtube.com/watch?v=NJd2u4fHXQA&list=PLvFHFPM09alKygQq-L6_6DwuNqTybIAw0&index=10
//10분정도 위치에 내용 있음
        if(HAL_UART_Receive_DMA(&huart3, (uint8_t *)&rx_buf[0],256) != HAL_OK)
        {
          ret = false;
        }
        qbuffer[ch].in = qbuffer[ch].len - hdma_usart3_rx.Instance->NDTR;
        qbuffer[ch].out = qbuffer[ch].in; //buffer는 flush 된다.(ring buffer 의미상)
#endif
#ifdef _USE_HW_UART_NODMA
        //IT  일때 이걸로
        if(HAL_UART_Receive_IT(&huart3, (uint8_t *)&rx_data[_DEF_UART2],1) != HAL_OK)
        {
          ret = false;
        }

#endif


       }
       break;
  }
  return ret;
}

uint32_t uartAvailable(uint8_t ch)
{
  uint32_t ret = 0;

  switch(ch)
  {
    case _DEF_UART1:
#ifdef _USE_HW_USB_CDC
      ret = cdcAvailable();
#endif

      break;
    case _DEF_UART2:
#ifdef _USE_HW_UART_DMA //DMA 일때
      qbuffer[ch].in = qbuffer[ch].len - hdma_usart3_rx.Instance->NDTR;
      //hdma_usart1_rx.Instance->NDTR 초기값은 256 으로 세팅되어 있는 상태
      //      //RM0090 참고
      //      //10.5.6 DMA stream x number of data register (DMA_SxNDTR) (x = 0..7)
#endif
      ret = qbufferAvailable(&qbuffer[ch]); //IT 모드일때 사용





      break;
  }

  return ret;
}
uint8_t  uartRead(uint8_t ch)
{
  uint8_t ret;

  switch(ch)
  {
    case _DEF_UART1:
#ifdef _USE_HW_USB_CDC
      ret = cdcRead();
#endif
      break;
    case _DEF_UART2:
      qbufferRead(&qbuffer[_DEF_UART2], &ret,1);
      break;
  }

  return ret;
}
uint32_t uartWrite(uint8_t ch, uint8_t *p_data,uint32_t length)
{
  uint32_t ret = 0;
  HAL_StatusTypeDef status;
  switch(ch)
  {
    case _DEF_UART1:
#ifdef _USE_HW_USB_CDC
      ret = cdcWrite(p_data, length);
#endif
      break;
    case _DEF_UART2:
      status = HAL_UART_Transmit(&huart3, p_data, length, 100);
      if(status == HAL_OK)
      {
        ret = length;
      }
      break;
//    case _DEF_UART3:
//      for(uint32_t i = 0; i < length; i++)
//      {
//        ITM_SendChar(p_data[i]);
//        delay(1);
//      }
//      ret = length;
//      break;
  }

  return ret;
}
uint32_t uartPrintf(uint8_t ch, char *fmt, ... )//... 은 가변인자
{
  char buf[256];
  va_list args;
  int len;

  va_start(args, fmt);
  len = vsnprintf(buf,256,fmt,args);

  uartWrite(ch, (uint8_t *)buf, len);
  va_end(args);
  return len;
}

uint32_t uartGetBaud(uint8_t ch)
{
  uint32_t ret = 0;
  switch(ch)
  {
    case _DEF_UART1:
#ifdef _USE_HW_USB_CDC
      ret = cdcGetBaud();
#endif
      break;
//    case _DEF_UART2:
//      ret = huart1.Init.BaudRate;
  }
  return ret;
}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(uartHandle->Instance==USART3)
  {
  /* USER CODE BEGIN USART3_MspInit 0 */

  /* USER CODE END USART3_MspInit 0 */
    /* USART3 clock enable */
    __HAL_RCC_USART3_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**USART3 GPIO Configuration
    PB10     ------> USART3_TX
    PB11     ------> USART3_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);


#ifdef _USE_HW_UART_DMA
    /* USART3 DMA Init */
    /* USART3_RX Init */
    hdma_usart3_rx.Instance = DMA1_Stream1;
    hdma_usart3_rx.Init.Channel = DMA_CHANNEL_4;
    hdma_usart3_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart3_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart3_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart3_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart3_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart3_rx.Init.Mode = DMA_CIRCULAR;
    hdma_usart3_rx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_usart3_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart3_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle,hdmarx,hdma_usart3_rx);
#endif
    /* USART3 interrupt Init */
    HAL_NVIC_SetPriority(USART3_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART3_IRQn);
  /* USER CODE BEGIN USART3_MspInit 1 */

  /* USER CODE END USART3_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==USART3)
  {
  /* USER CODE BEGIN USART3_MspDeInit 0 */

  /* USER CODE END USART3_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART3_CLK_DISABLE();

    /**USART3 GPIO Configuration
    PB10     ------> USART3_TX
    PB11     ------> USART3_RX
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_10|GPIO_PIN_11);

#ifdef _USE_HW_UART_DMA
    /* USART3 DMA DeInit */
    HAL_DMA_DeInit(uartHandle->hdmarx);
#endif
    /* USART3 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART3_IRQn);
  /* USER CODE BEGIN USART3_MspDeInit 1 */

  /* USER CODE END USART3_MspDeInit 1 */
  }
}



//void MX_USART1_UART_Init(void)
//{
//
//  /* USER CODE BEGIN USART1_Init 0 */
//
//  /* USER CODE END USART1_Init 0 */
//
//  /* USER CODE BEGIN USART1_Init 1 */
//
//  /* USER CODE END USART1_Init 1 */
//  huart1.Instance = USART1;
//  huart1.Init.BaudRate = 115200;
//  huart1.Init.WordLength = UART_WORDLENGTH_8B;
//  huart1.Init.StopBits = UART_STOPBITS_1;
//  huart1.Init.Parity = UART_PARITY_NONE;
//  huart1.Init.Mode = UART_MODE_TX_RX;
//  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
//  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
//  if (HAL_UART_Init(&huart1) != HAL_OK)
//  {
//    Error_Handler();
//  }
//  /* USER CODE BEGIN USART1_Init 2 */
//
//  /* USER CODE END USART1_Init 2 */
//
//}
//
//
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  if(huart->Instance == USART3)
  {
    while(1);
  }
}
//
//
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
#ifdef _USE_HW_UART_NODMA //DMA 사용시 필요 없음
  if(huart->Instance == USART3)
  {
    qbufferWrite(&qbuffer[_DEF_UART2],&rx_data[_DEF_UART2],1 );

    HAL_UART_Receive_IT(&huart3, (uint8_t *)&rx_data[_DEF_UART2],1);
  }
#endif
}

//void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
//{
//
//  GPIO_InitTypeDef GPIO_InitStruct = {0};
//  if(uartHandle->Instance==USART1)
//  {
//  /* USER CODE BEGIN USART1_MspInit 0 */
//
//  /* USER CODE END USART1_MspInit 0 */
//    /* USART1 clock enable */
//    __HAL_RCC_USART1_CLK_ENABLE();
//
//    __HAL_RCC_GPIOB_CLK_ENABLE();
//    /**USART1 GPIO Configuration
//    PB6     ------> USART1_TX
//    PB7     ------> USART1_RX
//    */
//    GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
//    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//    GPIO_InitStruct.Pull = GPIO_NOPULL;
//    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
//    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
//    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
//
//    /* USART1 DMA Init */
//    /* USART1_RX Init */
//    hdma_usart1_rx.Instance = DMA2_Stream2;
//    hdma_usart1_rx.Init.Channel = DMA_CHANNEL_4;
//    hdma_usart1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
//    hdma_usart1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
//    hdma_usart1_rx.Init.MemInc = DMA_MINC_ENABLE;
//    hdma_usart1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
//    hdma_usart1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
//    hdma_usart1_rx.Init.Mode = DMA_CIRCULAR;
//    hdma_usart1_rx.Init.Priority = DMA_PRIORITY_LOW;
//    hdma_usart1_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
//    if (HAL_DMA_Init(&hdma_usart1_rx) != HAL_OK)
//    {
//      Error_Handler();
//    }
//
//    __HAL_LINKDMA(uartHandle,hdmarx,hdma_usart1_rx);
//
//    /* USART1 interrupt Init */
//    HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
//    HAL_NVIC_EnableIRQ(USART1_IRQn);
//  /* USER CODE BEGIN USART1_MspInit 1 */
//
//  /* USER CODE END USART1_MspInit 1 */
//  }
//}

//void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
//{
//
//  if(uartHandle->Instance==USART1)
//  {
//  /* USER CODE BEGIN USART1_MspDeInit 0 */
//
//  /* USER CODE END USART1_MspDeInit 0 */
//    /* Peripheral clock disable */
//    __HAL_RCC_USART1_CLK_DISABLE();
//
//    /**USART1 GPIO Configuration
//    PB6     ------> USART1_TX
//    PB7     ------> USART1_RX
//    */
//    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6|GPIO_PIN_7);
//
//    /* USART1 DMA DeInit */
//    HAL_DMA_DeInit(uartHandle->hdmarx);
//
//    /* USART1 interrupt Deinit */
//    HAL_NVIC_DisableIRQ(USART1_IRQn);
//  /* USER CODE BEGIN USART1_MspDeInit 1 */
//
//  /* USER CODE END USART1_MspDeInit 1 */
//  }
//}
