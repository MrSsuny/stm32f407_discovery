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
  while(1)
  {
    if(millis()-pre_time >= 500)
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
    }

    if(uartAvailable(_DEF_UART2)>0)
    {
      uint8_t rx_data;

      rx_data = uartRead(_DEF_UART2);
      uartPrintf(_DEF_UART2, "Rx : 0x%X\n", rx_data);
    }
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
