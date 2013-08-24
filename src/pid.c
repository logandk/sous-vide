#include "pid.h"
#include "stm32f30x_timer.h"

// Interval
uint32_t interval_ = 1000; // [ms]

// Tuning
double k_p_;
double k_i_;
double k_d_;

// Limits
double min_;
double max_;

// Process variables
uint32_t last_time_;
double last_input_;
double target_;
double i_term_;
double output_;

uint8_t pid_compute(double input)
{
  uint32_t now     = timer_now();
  uint32_t delta_t = (now - last_time_);

  if (delta_t >= interval_)
  {
    // Compute all the working error variables
    double error = target_ - input;

    i_term_ += (k_i_ * error);

    if (i_term_ > max_) i_term_= max_;
    else if (i_term_ < min_) i_term_= min_;

    double delta_i = (input - last_input_);

    // Compute PID output
    output_ = k_p_ * error + i_term_ - k_d_ * delta_i;

    // Remember some variables for next time
    last_input_ = input;
    last_time_  = now;

    return 1;
  }

  return 0;
}

double pid_get_output()
{
  if (output_ > max_) output_ = max_;
  else if (output_ < min_) output_ = min_;

  return output_;
}

void pid_set_target(double target)
{
  target_ = target;
}

void pid_set_tuning(double k_p, double k_i, double k_d)
{
  double interval_s = (double) interval_ / 1000.0;

  k_p_ = k_p;
  k_i_ = k_i * interval_s;
  k_d_ = k_d / interval_s;
}

void pid_set_interval(uint32_t interval)
{
  if (interval > 0)
  {
    double ratio = (double) interval / (double) interval_;

    k_i_ *= ratio;
    k_d_ /= ratio;

    interval_ = interval;
  }
}

void pid_set_limits(double min, double max)
{
  if (max > min)
  {
    min_ = min;
    max_ = max;
  }
}

