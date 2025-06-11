/*
 * led.h
 *
 *  Created on: Jun 10, 2025
 *      Author: 2005b
 */

#ifndef SRC_COMMON_HW_INCLUDE_LED_H_
#define SRC_COMMON_HW_INCLUDE_LED_H_

//LED를 제어하기 위한 나만의 API
#include "hw_def.h" //하드웨어 관련된 정의가 들어있는 헤더 추가

//ifdef 부분은 내가 여러가지 하드웨어타입에서 작업할때 사용하기 위해서 만드는 부분
#ifdef _USE_HW_LED

//내가 작업하는 하드웨어 타입에 대한 정의를 가져가기 위한 부분
#define LED_MAX_CH            HW_LED_MAX_CH    //이렇게  해놓고 실제로 몇개의 LED를 사용할수 있는지를 hw_def.h 에서 HW_LED_MAX_CH에서 선언해주면 쉽다.

bool ledInit(void);  //초기화 함수
void ledOn(uint8_t ch);
void ledOff(uint8_t ch);
void ledToggle(uint8_t ch);

#endif

#endif /* SRC_COMMON_HW_INCLUDE_LED_H_ */
