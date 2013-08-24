/**
 * Ported from http://playground.arduino.cc/Code/PIDLibrary
 */

#ifndef PID_H
#define PID_H

#include "stm32f30x.h"

uint8_t pid_compute(double input);
double pid_get_output();
void pid_set_target(double target);
void pid_set_tuning(double k_p, double k_i, double k_d);
void pid_set_interval(uint32_t interval);
void pid_set_limits(double min, double max);

#endif
