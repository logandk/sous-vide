/**
 * Ported from http://playground.arduino.cc/Code/PIDAutotuneLibrary
 */

#ifndef AUTOTUNE_H
#define AUTOTUNE_H

#include "stm32f30x.h"

void autotune_init(double noiseband, double output_start, double output_step,
    double input_start, uint32_t interval);
uint8_t autotune_compute(double input);
double autotune_get_output();
double autotune_get_k_p();
double autotune_get_k_i();
double autotune_get_k_d();

void _autotune_calc_tuning();

#endif
