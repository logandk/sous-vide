#include "stm32f30x_sleep.h"

void sleep_ms(uint32_t time_ms)
{
  uint32_t time_us = time_ms * 1000;

  sleep_us(time_us);
}

void sleep_us(uint32_t time_us)
{
  // Set TIM clock
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

  // Initialize TIM timer
  TIM_TimeBaseInitTypeDef tim_init;

  tim_init.TIM_Prescaler     = 72 - 1;
  tim_init.TIM_Period        = 65535;
  tim_init.TIM_ClockDivision = 0;
  tim_init.TIM_CounterMode   = TIM_CounterMode_Down;

  TIM_TimeBaseInit(TIM2, &tim_init);

  // Start timer
  TIM_Cmd(TIM2, ENABLE);
  TIM_SetCounter(TIM2, time_us);

  // Wait
  while (time_us)
  {
    time_us = TIM_GetCounter(TIM2);
  }

  // Disable timer
  TIM_Cmd(TIM2, DISABLE);
}

