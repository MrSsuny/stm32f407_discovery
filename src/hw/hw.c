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

  flashInit();
  sdInit();
}
