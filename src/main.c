#include "stm32f30x_lcd.h"
#include "stm32f30x_temp.h"
#include "stm32f30x_sleep.h"

int main(void)
{
  temp_def temperature;

  temperature.port = GPIOC;
  temperature.pin = GPIO_Pin_6;

  uint16_t temp = 0;
  temp = temp_read(&temperature);

  lcd_def display;

  display.port = GPIOD;
  display.pin_enable = GPIO_Pin_8;
  display.pin_register_select = GPIO_Pin_9;
  display.pin_d4 = GPIO_Pin_13;
  display.pin_d5 = GPIO_Pin_12;
  display.pin_d6 = GPIO_Pin_11;
  display.pin_d7 = GPIO_Pin_10;

  lcd_init(&display);
  lcd_printf(&display, "det virker!");
  lcd_set_pos(&display, 2, 1);
  lcd_printf(&display, "hej %c", 0xdf);
  int i = 1;
  while (1)
  {
    temp = temp_read(&temperature);

    //lcd_clear(&display);
    lcd_home(&display);
    //uint16_t temp = 20435;

    float temp2 = (float) temp * 62.5;

    lcd_printf(&display, "okay? %d%c", (uint32_t) temp2, 0xdf);
    lcd_set_pos(&display, 2, 1);
    lcd_printf(&display, "hej %d%c", i * 10000, 0xdf);
    sleep_ms(500);

    i++;
  }
}

