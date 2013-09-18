#include "stm32f3_discovery.h"
#include "stm32f30x_lcd.h"
#include "stm32f30x_temp.h"
#include "stm32f30x_sleep.h"
#include "stm32f30x_timer.h"
#include "pid.h"
#include <math.h>

#define DUTY_CYCLE  5000

lcd_def display;
double cur_temp, set_temp;
uint32_t output;
uint8_t state, critical;
uint8_t spinner = 0xa5;

void relay_init()
{
  // Configure relay
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

  GPIO_InitTypeDef gpio_init;

  gpio_init.GPIO_Pin   = GPIO_Pin_9;
  gpio_init.GPIO_Mode  = GPIO_Mode_OUT;
  gpio_init.GPIO_OType = GPIO_OType_PP;
  gpio_init.GPIO_Speed = GPIO_Speed_10MHz;
  gpio_init.GPIO_PuPd  = GPIO_PuPd_NOPULL;

  GPIO_Init(GPIOA, &gpio_init);
  GPIO_ResetBits(GPIOA, GPIO_Pin_9);
}

void display_state(lcd_def *display)
{
  // Clear display
  lcd_home(display);

  // Split at decimal point
  double f_part, i_part;
  int target_i, target_f;
  int current_i, current_f;

  f_part = modf(set_temp, &i_part);
  target_i = (int) i_part;
  target_f = (int) (f_part * 100);

  f_part = modf(cur_temp, &i_part);
  current_i = (int) i_part;
  current_f = (int) (f_part * 100);

  // Print current temperature
  lcd_printf(display, "%c%02d.%02d%c  %c%02d.%02d%c",
      spinner, current_i, current_f, 0xdf,
      0x7e, target_i, target_f, 0xdf);

  lcd_set_pos(display, 0, 1);
  lcd_printf(display, "Duty cycle: %4d", output);

  switch (spinner)
  {
    case 0xa5:
      spinner = 0x20;
      break;

    case 0x20:
      spinner = 0xa5;
      break;
  }
}

void save_state()
{
  FLASH_Unlock();

  uint32_t address = 0x08006000;
  uint32_t value   = (uint32_t) (set_temp * 100.0);

  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);
  FLASH_ErasePage(address);
  FLASH_ProgramWord(address, value);

  FLASH_Lock();
}

void load_state()
{
  uint32_t address = 0x08006000;
  uint32_t value   = *(__IO uint32_t *)address;

  set_temp = (double) value * 0.01;
}

void EXTI0_IRQHandler(void)
{
  if (EXTI_GetITStatus(USER_BUTTON_EXTI_LINE) == SET && STM_EVAL_PBGetState(BUTTON_USER) != RESET)
  {
    if (!critical)
    {
      sleep_ms(100);

      set_temp += 0.5;
      if (set_temp > 100.0) set_temp = 20.0;

      // Write to display
      display_state(&display);

      sleep_ms(500);

      // Wait for SEL button to be pressed
      while(STM_EVAL_PBGetState(BUTTON_USER) != RESET)
      {
        set_temp += 0.5;
        if (set_temp > 100.0) set_temp = 20.0;

        // Set PID target
        pid_set_target(set_temp);

        // Write to display
        display_state(&display);

        sleep_ms(100);
      }

      save_state();
    }

    // Clear the EXTI line pending bit
    EXTI_ClearITPendingBit(USER_BUTTON_EXTI_LINE);
  }
}

int main(void)
{
  set_temp = 50.0;
  cur_temp = 0.0;
  critical = 0;
  state    = 0;

  // Reload state
  load_state();

  // Initialize push button
  STM_EVAL_PBInit(BUTTON_USER, BUTTON_MODE_EXTI);

  // Initialize systick timer
  timer_init();

  // Configure temperature sensor DS18B20
  temp_def temperature;
  temperature.port = GPIOC;
  temperature.pin = GPIO_Pin_6;

  // Configure LCD display HD44780
  display.port = GPIOD;
  display.pin_enable = GPIO_Pin_8;
  display.pin_register_select = GPIO_Pin_9;
  display.pin_d4 = GPIO_Pin_13;
  display.pin_d5 = GPIO_Pin_12;
  display.pin_d6 = GPIO_Pin_11;
  display.pin_d7 = GPIO_Pin_10;
  lcd_init(&display);

  // Configure PID control
  pid_set_target(set_temp);
  pid_set_tuning(3, 10, 4);
  pid_set_interval(1000);
  pid_set_limits(0, DUTY_CYCLE);

  // Initialize relay
  relay_init();

  // Set cycle start time
  uint32_t cycle_start = timer_now();

  // Loop forever
  while (1)
  {
    // Check if temperature reading is ready
    if (temp_ready(&temperature))
    {
      // Convert temperature to degrees celcius
      critical = 1;
      cur_temp = (double) temp_read(&temperature) * 0.0625;
      critical = 0;
    }

    // Calculate with new parameters
    if (pid_compute(cur_temp))
    {
      critical = 1;
      output = (uint32_t) pid_get_output();

      // Write to display
      display_state(&display);

      // Initiate next temperature reading
      temp_convert(&temperature);
      critical = 0;
    }


    // Check if a new cycle should be started
    uint32_t delta_t = timer_now() - cycle_start;

    if (delta_t >= DUTY_CYCLE)
    {
      cycle_start = timer_now();
    }

    // Toggle relay
    if (output > delta_t && state == 0)
    {
      // Turn on relay
      GPIO_SetBits(GPIOA, GPIO_Pin_9);
      state = 1;
    }
    else if (state == 1 && output != DUTY_CYCLE)
    {
      // Turn off relay
      GPIO_ResetBits(GPIOA, GPIO_Pin_9);
      state = 0;
    }
  }
}

