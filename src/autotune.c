#include "autotune.h"
#include "stm32f30x_timer.h"
#include <math.h>

// Configuration
static double noiseband_;
static double output_start_;
static double output_step_;
static double input_start_;
static uint32_t interval_;

// Tuning parameters
static double k_u_, p_u_;

// Process variables
static uint8_t lookback_;
static uint32_t comp_count_;
static double output_;
static double output_step_;
static double abs_max_, abs_min_;
static uint32_t last_time_;
static uint8_t just_changed_;
static uint8_t peak_type_, peak_count_;
static uint32_t peak_a_, peak_b_;
static double peaks_[10], last_inputs_[10];

void autotune_init(double noiseband, double output_start, double output_step,
    double input_start, uint32_t interval)
{
  noiseband_    = noiseband;
  output_start_ = output_start;
  output_step_  = output_step;
  input_start_  = input_start;
  interval_     = interval;

  lookback_     = 9;
  comp_count_   = 0;
  last_time_    = timer_now();
  peak_type_    = 0;
  peak_count_   = 0;
  just_changed_ = 0;
  abs_max_      = input_start;
  abs_min_      = input_start;
  output_       = output_start_ + output_step_;
}

uint8_t autotune_compute(double input)
{
  if (peak_count_ > 9)
  {
    _autotune_calc_tuning();
    return 2;
  }

  uint32_t now = timer_now();

  if ((now - last_time_) < interval_)
  {
    return 0;
  }

  last_time_ = now;

  if (input > abs_max_)
  {
    abs_max_ = input;
  }

  if (input < abs_min_)
  {
    abs_min_ = input;
  }

  // oscillate the output base on the input's relation to the input_start_
  if (input > input_start_ + noiseband_)
  {
    output_ = output_start_ - output_step_;
  }
  else if (input < input_start_ - noiseband_)
  {
    output_ = output_start_ + output_step_;
  }

  uint8_t is_max = 1;
  uint8_t is_min = 1;

  for (uint32_t i = lookback_ - 1; i; i--)
  {
    double val = last_inputs_[i];

    if (is_max)
    {
      is_max = input > val;
    }

    if (is_min)
    {
      is_min = input < val;
    }

    last_inputs_[i + 1] = last_inputs_[i];
  }

  last_inputs_[0] = input;
  comp_count_++;

  if (comp_count_ < 10)
  {
    // we don't want to trust the maxes or mins until the inputs array has been filled
    return 1;
  }

  if (is_max)
  {
    if (peak_type_ == 0)
    {
      peak_type_ = 1;
    }

    if (peak_type_ == 2)
    {
      peak_type_ = 1;
      just_changed_ = 1;
      peak_b_ = peak_a_;
    }

    peak_a_ = now;
    peaks_[peak_count_] = input;

  }
  else if (is_min)
  {
    if (peak_type_ == 0)
    {
      peak_type_ = 2;
    }

    if (peak_type_ == 1)
    {
      peak_type_ = 2;
      peak_count_++;
      just_changed_ = 1;
    }

    if (peak_count_ < 10)
    {
      peaks_[peak_count_] = input;
    }
  }

  if (just_changed_ && peak_count_ > 2)
  {
    // we've transitioned.  check if we can autotune based on the last peaks_
    double avg_sep = (fabs(peaks_[peak_count_ - 1] - peaks_[peak_count_ - 2]) +
        fabs(peaks_[peak_count_ - 2] - peaks_[peak_count_ - 3])) / 2.0;

    if (avg_sep < 0.05 * (abs_max_ - abs_min_))
    {
      _autotune_calc_tuning();
      return 2;

    }
  }

  just_changed_ = 0;
  return 1;
}

double autotune_get_output()
{
  return output_;
}

double autotune_get_k_p()
{
  return 0.6 * k_u_;
}

double autotune_get_k_i()
{
  return 1.2 * k_u_ / p_u_; // Ki = Kc/Ti
}

double autotune_get_k_d()
{
  return 0.075 * k_u_ * p_u_; //Kd = Kc * Td
}

void _autotune_calc_tuning()
{
  output_ = output_start_;

  //we can generate tuning parameters!
  k_u_ = 4 * (2 * output_step_) / ((abs_max_ - abs_min_) * 3.14159);
  p_u_ = (double) (peak_a_ - peak_b_) / 1000.0;
}

