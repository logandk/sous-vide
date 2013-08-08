#include "stm32f30x_lcd.h"
#include "stm32f30x_sleep.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

void lcd_init(const lcd_def *def)
{
  // Set GPIO clock
  if (def->port == GPIOA)
  {
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
  }
  else if (def->port == GPIOB)
  {
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
  }
  else if (def->port == GPIOC)
  {
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);
  }
  else if (def->port == GPIOD)
  {
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOD, ENABLE);
  }
  else if (def->port == GPIOE)
  {
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOE, ENABLE);
  }
  else if (def->port == GPIOF)
  {
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOF, ENABLE);
  }

  // Initialize GPIO pins
  GPIO_InitTypeDef gpio_init;

  gpio_init.GPIO_Pin   = def->pin_register_select |
                         def->pin_enable |
                         def->pin_d4 |
                         def->pin_d5 |
                         def->pin_d6 |
                         def->pin_d7;

  gpio_init.GPIO_Mode  = GPIO_Mode_OUT;
  gpio_init.GPIO_OType = GPIO_OType_PP;
  gpio_init.GPIO_Speed = GPIO_Speed_10MHz;
  gpio_init.GPIO_PuPd  = GPIO_PuPd_NOPULL;

  GPIO_Init(def->port, &gpio_init);

  // wait for initialization
  sleep_ms(30);

  // select 4-bit interface
  _lcd_set_nibble(def, 2);
  sleep_ms(20);

  // init
  _lcd_send_command(def, 0x28); // 4 bit mode, 2 lines, 5x7 glyph
  _lcd_send_command(def, 0x0c); // display with no cursor, no blink
  _lcd_send_command(def, 0x06); // automatic increment, no display shift

  lcd_home(def);
  lcd_clear(def);
}

void lcd_clear(const lcd_def *def)
{
  _lcd_send_command(def, 0x01);
}

void lcd_home(const lcd_def *def)
{
  _lcd_send_command(def, 0x80);
  sleep_ms(1);
}

void lcd_set_pos(const lcd_def *def, uint8_t x, uint8_t y)
{
  uint8_t pos = x | 0x80;

  // add y position
  if (y == 2)
  {
    pos |= 0x14; // line 2
  }
  else
  {
    pos |= 0x40; // line 1
  }

  _lcd_send_command(def, pos);
}

void lcd_printf(const lcd_def *def, const char *format, ...)
{
  va_list args;
  char out[32];

  // format output
  va_start(args, format);
  vsprintf(out, format, args);
  va_end(args);

  // write characters
  int len = strlen(out);

  for(uint8_t i = 0; i < len; i++)
  {
    _lcd_send_data(def, out[i]);
  }
}

void _lcd_pin_up(const lcd_def *def, uint16_t pin)
{
  def->port->ODR |= pin;
}

void _lcd_pin_down(const lcd_def *def, uint16_t pin)
{
  def->port->BRR |= pin;
}

void _lcd_set_nibble(const lcd_def *def, uint8_t data)
{
  _lcd_pin_up(def, def->pin_enable);

  (data & (1 << 0)) ? _lcd_pin_up(def, def->pin_d4) : _lcd_pin_down(def, def->pin_d4);
  (data & (1 << 1)) ? _lcd_pin_up(def, def->pin_d5) : _lcd_pin_down(def, def->pin_d5);
  (data & (1 << 2)) ? _lcd_pin_up(def, def->pin_d6) : _lcd_pin_down(def, def->pin_d6);
  (data & (1 << 3)) ? _lcd_pin_up(def, def->pin_d7) : _lcd_pin_down(def, def->pin_d7);

  _lcd_pin_down(def, def->pin_enable);

  sleep_ms(1);
}

void _lcd_set_byte(const lcd_def *def, uint8_t data)
{
  // send high nibble
  _lcd_set_nibble(def, (data >> 4) & 0x0f);

  // send low nibble
  _lcd_set_nibble(def, data & 0x0f);
}

void _lcd_send_command(const lcd_def *def, uint8_t data)
{
  _lcd_pin_down(def, def->pin_register_select);
  _lcd_set_byte(def, data);
  sleep_ms(5);
}

void _lcd_send_data(const lcd_def *def, uint8_t data)
{
  _lcd_pin_up(def, def->pin_register_select);
  _lcd_set_byte(def, data);
  sleep_ms(1);
}

