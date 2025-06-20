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
  cliInit();
  ledInit();
  gpioInit();

  //flashInit();
  if(sdInit() == true)
  {
    fatfsInit();
  }
  usbBegin(USB_CDC_MODE);
}
