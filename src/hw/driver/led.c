/*
 * led.c
 *
 *  Created on: Jun 10, 2025
 *      Author: 2005b
 */
#include "led.h"
//#include "cli.h"

typedef struct
{
  GPIO_TypeDef* port;
  uint16_t      pin;
  GPIO_PinState on_state;
  GPIO_PinState off_state;
}led_tbl_t;

led_tbl_t led_tbl[LED_MAX_CH] =
{
    {GPIOD, GPIO_PIN_12, GPIO_PIN_SET, GPIO_PIN_RESET},
    {GPIOD, GPIO_PIN_13, GPIO_PIN_SET, GPIO_PIN_RESET},
    {GPIOD, GPIO_PIN_14, GPIO_PIN_SET, GPIO_PIN_RESET},
    {GPIOD, GPIO_PIN_15, GPIO_PIN_SET, GPIO_PIN_RESET},
};
#ifdef _USE_HW_CLI
static void cliLed(cli_args_t *args);

#endif
bool ledInit(void)
{
  bool ret = true;
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();


  /*Configure GPIO pins : PD12 PD13 PD14 PD15 */

  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
  for(int i=0;i<LED_MAX_CH;i++)
  {
    GPIO_InitStruct.Pin = led_tbl[i].pin;
    //위에 코드에서 이 부분 대체함
    //GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
    HAL_GPIO_Init(led_tbl[i].port, &GPIO_InitStruct);
    //default 로는 LED를 off 시키는 게 좋으니깐
    ledOff(i);
    //위에 코드에서 이 부분을 대체 한것이다.  LED Off 한거
    //HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);

  }
#ifdef _USE_HW_CLI
  cliAdd("led",cliLed);
#endif
  return ret;
}
void ledOn(uint8_t ch)
{
  if(ch >= LED_MAX_CH) return;
  HAL_GPIO_WritePin(led_tbl[ch].port,led_tbl[ch].pin,led_tbl[ch].on_state);
}
void ledOff(uint8_t ch)
{
  if(ch >= LED_MAX_CH) return;
    HAL_GPIO_WritePin(led_tbl[ch].port,led_tbl[ch].pin,led_tbl[ch].off_state);
}
void ledToggle(uint8_t ch)
{
  if(ch >= LED_MAX_CH) return;
  HAL_GPIO_TogglePin(led_tbl[ch].port,led_tbl[ch].pin);
}




