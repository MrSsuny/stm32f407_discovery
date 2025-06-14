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
void apInit(void)
{
  uartOpen(_DEF_UART1,115200);
  uartOpen(_DEF_UART2,115200);
}

void apMain(void)
{
  uint32_t pre_time;
  delay(500);
  pre_time = millis();
  uartPrintf(_DEF_UART1,"USB UART Main %d\n",millis());
  while(1)
  {
    if(millis()-pre_time >= 500)
    {
      pre_time = millis();
      ledToggle(_DEF_LED1);
      //debugPrint("debug printf %d\n", (int)pre_time);
      //logPrintf("printf Test %d\n", (int)pre_time);
      //strlen
      uartPrintf(_DEF_UART2,"Uart1 %d\n",millis());
      uartPrintf(_DEF_UART1,"USB UART Loop %d\n",millis());
    }

    //HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12);
    //CDC_Transmit_FS("test\n",6);
    //delay(500);
    if (uartAvailable(_DEF_UART2) > 0)
    {

      uint8_t rx_data;
      //rx_data = cdcRead();
      rx_data = uartRead(_DEF_UART2);
      uartPrintf(_DEF_UART2,"UART1 : %c 0x%X\n", rx_data,rx_data);
      uartPrintf(_DEF_UART1,"USB UART Received 0x%X\n",rx_data);
//      cdcWrite((uint8_t *)"RxData : ", 10);
//      cdcWrite(&rx_data,1);
//      cdcWrite((uint8_t *)"\n",2);
    }
  }
}
