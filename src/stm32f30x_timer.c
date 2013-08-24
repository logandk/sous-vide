#include "stm32f30x_timer.h"

static volatile uint32_t timer_count_;

void timer_init()
{
  timer_count_ = 0;

  if (SysTick_Config(SystemCoreClock / 1000))
  {
    // Capture error
    while (1);
  }
}

uint32_t timer_now()
{
  return timer_count_;
}

void SysTick_Handler(void)
{
  timer_count_++;
}
