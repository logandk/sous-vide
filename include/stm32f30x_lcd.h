/**
 * LCD HD44780 driver for STM32 F3 Discovery. Uses 4-bit interface
 * for communication.
 *
 * Wiring:
 *  1. ground
 *  2. +5V (supply)
 *  3. 2.7k ohm to supply (contrast)
 *  4. pin_register_select
 *  5. ground (read/write)
 *  6. pin_enable
 *  7. -
 *  8. -
 *  9. -
 * 10. -
 * 11. pin_d4 (data)
 * 12. pin_d5 (data)
 * 13. pin_d6 (data)
 * 14. pin_d7 (data)
 * 15. 670 ohm to supply (led anode)
 * 16. ground (led cathode)
 */

#ifndef STM32F30X_LCD_H
#define STM32F30X_LCD_H

#include "stm32f30x.h"

// LCD definition
typedef struct
{
  GPIO_TypeDef *port;

  uint16_t pin_register_select;
  uint16_t pin_enable;

  uint16_t pin_d4;
  uint16_t pin_d5;
  uint16_t pin_d6;
  uint16_t pin_d7;

} lcd_def;

// Public interface
void lcd_init(const lcd_def *def);
void lcd_clear(const lcd_def *def);
void lcd_home(const lcd_def *def);
void lcd_set_pos(const lcd_def *def, uint8_t x, uint8_t y);
void lcd_printf(const lcd_def *def, const char *format, ...);

// Internal functions
void _lcd_pin_up(const lcd_def *def, uint16_t pin);
void _lcd_pin_down(const lcd_def *def, uint16_t pin);
void _lcd_set_nibble(const lcd_def *def, uint8_t data);
void _lcd_set_byte(const lcd_def *def, uint8_t data);
void _lcd_send_command(const lcd_def *def, uint8_t data);
void _lcd_send_data(const lcd_def *def, uint8_t data);

#endif

