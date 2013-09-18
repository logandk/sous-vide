/**
 * Temperature sensor DS18B20 driver for STM32 F3 Discovery. Uses 1-wire
 * interface for communication.
 *
 * Wiring:
 *  1. (black) ground
 *  2. (red) +5V (supply)
 *  3. (blue) 4.7k ohm to supply (data)
 */

#ifndef STM32F30X_TEMP_H
#define STM32F30X_TEMP_H

#include "stm32f30x.h"

// temp definition
typedef struct
{
  GPIO_TypeDef *port;
  uint16_t pin;
} temp_def;

// Public interface
void temp_convert(const temp_def *def);
uint8_t temp_ready(const temp_def *def);
uint16_t temp_read(const temp_def *def);

// Internal functions
void _temp_pin_up(const temp_def *def);
void _temp_pin_down(const temp_def *def);
void _temp_pin_init(const temp_def *def, GPIOMode_TypeDef mode);
void _temp_pin_in(const temp_def *def);
void _temp_pin_out(const temp_def *def);
void _temp_init(const temp_def *def);
uint8_t _temp_check(const temp_def *def);
uint8_t _temp_read(const temp_def *def);
void _temp_write(const temp_def *def, uint8_t data);

#endif

