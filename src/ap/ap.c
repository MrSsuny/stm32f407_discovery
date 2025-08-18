/*
 * ap.c
 *
 *  Created on: Jun 7, 2025
 *      Author: 2005b
 */


#include "ap.h"

extern uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len);
extern uint32_t cdcAvailable(void);
extern uint8_t cdcRead(void);
extern void cdcDatatIn(uint8_t rx_data);
extern uint32_t cdcWrite(uint8_t *p_data, uint32_t length);
extern uint32_t sof_count;
extern bool firstPulseDetected;
extern volatile int tim6Count;
//extern TIM_HandleTypeDef htim2;

/* 최대 20펄스, High/Low 시간(µs) 저장 */
volatile uint32_t highTime[20];
volatile uint32_t  lowTime[20];
volatile uint8_t    pulseCount = 0;

/* 캡처 타임스탬프 */
volatile uint32_t lastCapTime = 0;

/* 이벤트 구분용 타이머(ms) */
volatile uint32_t lastEventTick = 0;

void cliModem(cli_args_t *args);

void apInit(void)
{
  uartOpen(_DEF_UART1,115200);
  uartOpen(_DEF_UART2,115200);
#ifdef _USE_HW_CLI
  cliOpen(_DEF_UART1, 115200);
  cliOpenLog(_DEF_UART2, 115200);
  cliAdd("modem",cliModem);
#endif
}

void apMain(void)
{
  uint32_t pre_time;
  pre_time = millis();
  //
#ifdef _USE_HW_USB_CDC
  uartPrintf(_DEF_UART1,"USB UART1 Main %d\n",millis());
#endif
//  HAL_Delay(50);
//  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET);
//  HAL_Delay(50);
//  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_SET);
//  HAL_Delay(50);
//  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET);
//  HAL_Delay(50);
//  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_SET);
//  HAL_Delay(50);
//  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET);
//  HAL_Delay(50);
//  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_SET);
//  HAL_Delay(50);

  TIM_IC_Start_IT(&htim2, TIM_CHANNEL_2);
  //__HAL_TIM_DISABLE_IT(&htim6, TIM_IT_UPDATE);  // 일단 끄고

  /* 초기화 */
  lastEventTick = HAL_GetTick();
  while(1)
  {
    if(millis()-pre_time >= 1000)
    {
      pre_time = millis();
      ledToggle(_DEF_LED1);
#ifdef _USE_HW_USB_CDC
  //uartPrintf(_DEF_UART2,"Uart1 %d\n",(int)millis());
  //uartPrintf(_DEF_UART1,"USB UART1 %d\n",(int)millis());
#endif
      //debugPrint("debug printf %d\n", (int)pre_time);
      //logPrintf("printf Test %d\n", (int)pre_time);
      //strlen
      //uartPrintf(_DEF_UART2,"Uart1 %d\n",(int)millis());
      //uartPrintf(_DEF_UART1,"USB UART Loop %d\n",(int)millis());
      //logPrintf("logPrintf %d\n", (int)millis());
     // uartPrintf(_DEF_UART2,"tim6Count %d\n",(int)tim6Count);
      //logPrintf("tim6Count %d\n", (int)tim6Count);

    }

    if(uartAvailable(_DEF_UART2)>0)
    {
      uint8_t rx_data;

      rx_data = uartRead(_DEF_UART2);
      uartPrintf(_DEF_UART2, "Rx : 0x%X\n", rx_data);
    }
    timerEventCheck();
#ifdef _USE_HW_CLI
    cliMain();
#endif
  }
}

void cliModem(cli_args_t *args)
{
  bool ret = false;
  bool keep_loop;
  ymodem_t ymodem;


  if (args->argc == 2 && args->isStr(0, "down"))
  {
    uint32_t addr_offset;
    uint32_t addr;

    addr_offset = args->getData(1);

    keep_loop = true;

    ymodemOpen(&ymodem, _DEF_UART1);

    while(keep_loop)
    {
      if (ymodemReceive(&ymodem) == true)
      {
        switch(ymodem.type)
        {
          case YMODEM_TYPE_START:
            flashErase(addr_offset, ymodem.file_length);
            break;

          case YMODEM_TYPE_DATA:
            addr = addr_offset + ymodem.file_addr;
            flashWrite(addr, ymodem.file_buf, ymodem.file_buf_length);
            break;

          case YMODEM_TYPE_END:
            keep_loop = false;
            break;

          case YMODEM_TYPE_CANCEL:
            keep_loop = false;
            break;

          case YMODEM_TYPE_ERROR:
            keep_loop = false;
            break;
        }
      }
    }

    if (ymodem.type == YMODEM_TYPE_END)
    {
      cliPrintf("Down OK\n");
    }
    else
    {
      cliPrintf("Down Fail\n");
    }
    ret = true;
  }

  if (ret != true)
  {
    cliPrintf("modem down [addr] \n");
  }
}
