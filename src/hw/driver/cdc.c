/*
 * cdc.c
 *
 *  Created on: Jun 11, 2025
 *      Author: 2005b
 */


#include "cdc.h"

#ifdef _USE_HW_USB_CDC

static bool is_init = false;

bool cdcInit(void)
{
  bool ret = true;
  is_init = true;
  return ret;
}
bool cdcIsInit(void)
{
  return is_init;
}

#endif

