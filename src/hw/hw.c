/*
 * hw.c
 *
 *  Created on: Jun 7, 2025
 *      Author: 2005b
 */



#include "hw.h"


void hwInit(void)
{
  bspInit();
  usbInit(); //uart init 보다 먼저 와야한다.
  uartInit();
#ifdef _USE_HW_CLI
  cliInit();
#endif
  ledInit();
  gpioInit();

  flashInit();
  if(sdInit() == true)
  {
    fatfsInit();
  }
  if(gpioPinRead(1) == true)
  {
    //uartPrintf(_DEF_UART2, "SW High\n");
    usbBegin(USB_MSC_MODE);
  }
  else
  {
    //uartPrintf(_DEF_UART2, "SW Low(default)\n");
    usbBegin(USB_CDC_MODE);
  }


}
