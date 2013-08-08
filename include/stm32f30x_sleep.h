/**
 * Sleep function for STM32 F3 Discovery
 */

#ifndef STM32F30X_SLEEP_H
#define STM32F30X_SLEEP_H

#include "stm32f30x.h"

// Public interface
void sleep_ms(uint32_t time_ms);
void sleep_us(uint32_t time_us);

#endif

