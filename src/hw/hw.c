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
  cliInit();
  ledInit();
  uartInit();

  flashInit();
}
