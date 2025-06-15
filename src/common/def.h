/*
 * def.h
 *
 *  Created on: Jun 7, 2025
 *      Author: 2005b
 */

#ifndef SRC_COMMON_DEF_H_
#define SRC_COMMON_DEF_H_
//모두 사용하는 헤더
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h> //cli 사용
#include <stdlib.h> //cli 사용

#define _DEF_LED1         0
#define _DEF_LED2         1
#define _DEF_LED3         2
#define _DEF_LED4         3

#define _DEF_UART1        0
#define _DEF_UART2        1
#define _DEF_UART3        2
#define _DEF_UART4        3


#define _DEF_INPUT            0
#define _DEF_INPUT_PULLUP     1
#define _DEF_INPUT_PULLDOWN   2
#define _DEF_OUTPUT           3
#define _DEF_OUTPUT_PULLUP    4
#define _DEF_OUTPUT_PULLDOWN  5

#define _DEF_LOW              0
#define _DEF_HIGH             1

#endif /* SRC_COMMON_DEF_H_ */
