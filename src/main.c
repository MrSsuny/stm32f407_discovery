/*
 * main.c
 *
 *  Created on: Jun 7, 2025
 *      Author: 2005b
 */


#include "main.h"
#include "stm32f4xx_hal.h"




int main(void)
{
  hwInit();
  apInit();
  apMain();
}
