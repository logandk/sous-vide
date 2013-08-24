#include "stm32f30x_lcd.h"
#include "stm32f30x_temp.h"
#include "stm32f30x_sleep.h"
#include "stm32f30x_timer.h"
#include "pid.h"
#include <math.h>

#define TARGET_TEMP 34
#define DUTY_CYCLE  5000

int main(void)
{
  // Initialize systick timer
  timer_init();

  // Configure temperature sensor DS18B20
  temp_def temperature;
  temperature.port = GPIOC;
  temperature.pin = GPIO_Pin_6;

  // Configure LCD display HD44780
  lcd_def display;
  display.port = GPIOD;
  display.pin_enable = GPIO_Pin_8;
  display.pin_register_select = GPIO_Pin_9;
  display.pin_d4 = GPIO_Pin_13;
  display.pin_d5 = GPIO_Pin_12;
  display.pin_d6 = GPIO_Pin_11;
  display.pin_d7 = GPIO_Pin_10;
  lcd_init(&display);

  // Configure PID control
  pid_set_target(TARGET_TEMP);
  pid_set_tuning(2, 5, 1);
  pid_set_interval(1000);
  pid_set_limits(0, DUTY_CYCLE);

  // Configure relay
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

  GPIO_InitTypeDef gpio_init;

  gpio_init.GPIO_Pin   = GPIO_Pin_8;

  gpio_init.GPIO_Mode  = GPIO_Mode_OUT;
  gpio_init.GPIO_OType = GPIO_OType_PP;
  gpio_init.GPIO_Speed = GPIO_Speed_10MHz;
  gpio_init.GPIO_PuPd  = GPIO_PuPd_NOPULL;

  GPIO_Init(GPIOA, &gpio_init);
  GPIO_ResetBits(GPIOA, GPIO_Pin_8);

  // Set cycle start time
  uint32_t cycle_start = timer_now();

  // Loop forever
  while (1)
  {
    // Convert temperature to degrees celcius
    double temp = temp_read(&temperature);
    temp *= 0.0625;

    // Calculate with new parameters
    pid_compute(temp);
    uint32_t output = (uint32_t) pid_get_output();

    uint32_t delta_t = timer_now() - cycle_start;

    if (delta_t > DUTY_CYCLE)
    {
      cycle_start += DUTY_CYCLE;
      delta_t     -= DUTY_CYCLE;
    }

    if (output > delta_t)
    {
      // Turn on relay
      GPIO_SetBits(GPIOA, GPIO_Pin_8);
    }
    else
    {
      // Turn off relay
      GPIO_ResetBits(GPIOA, GPIO_Pin_8);
    }

    // Clear display
    lcd_home(&display);

    // Split at decimal point
    double f_part, i_part;
    int target_i, target_f;
    int current_i, current_f;

    f_part = modf(TARGET_TEMP, &i_part);
    target_i = (int) i_part;
    target_f = (int) (f_part * 100);

    f_part = modf(temp, &i_part);
    current_i = (int) i_part;
    current_f = (int) (f_part * 100);

    // Print current temperature
    lcd_printf(&display, "%c%02d.%02d%c  %c%02d.%02d%c",
        0xa5, current_i, current_f, 0xdf,
        0x7e, target_i, target_f, 0xdf);

    lcd_set_pos(&display, 0, 1);
    lcd_printf(&display, "Duty cycle: %d", output);
  }
}

