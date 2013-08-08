#include "stm32f30x_temp.h"
#include "stm32f30x_sleep.h"

uint16_t temp_read(const temp_def *def)
{
  uint16_t result;

  // Initiate a temperature conversion
  _temp_init(def);
  _temp_write(def, 0xcc); // Skip ROM
  _temp_write(def, 0x44); // Convert T

  while (!_temp_read(def)); // Wait for conversion

  _temp_init(def);
  _temp_write(def, 0xcc); // Skip ROM
  _temp_write(def, 0xbe); // Read scratchpad

  result = _temp_read(def);
  result += ((uint16_t) _temp_read(def)) << 8;

  return result;
}

void _temp_pin_up(const temp_def *def)
{
  def->port->ODR |= def->pin;
}

void _temp_pin_down(const temp_def *def)
{
  def->port->BRR |= def->pin;
}

void _temp_pin_init(const temp_def *def, GPIOMode_TypeDef mode)
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

  gpio_init.GPIO_Pin   = def->pin;
  gpio_init.GPIO_Mode  = mode;
  gpio_init.GPIO_OType = GPIO_OType_PP;
  gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
  gpio_init.GPIO_PuPd  = GPIO_PuPd_NOPULL;

  GPIO_Init(def->port, &gpio_init);
}

void _temp_pin_in(const temp_def *def)
{
  _temp_pin_init(def, GPIO_Mode_IN);
}

void _temp_pin_out(const temp_def *def)
{
  _temp_pin_init(def, GPIO_Mode_OUT);
}

void _temp_init(const temp_def *def)
{
  // Send initialization sequence
  _temp_pin_out(def);
  _temp_pin_down(def);
  sleep_us(500); // Pull low for minimum 480 us
  _temp_pin_in(def);
  sleep_us(80); // Wait 15 to 60 us
  while (_temp_check(def));
  _temp_pin_out(def);
  _temp_pin_up(def);
  sleep_us(400); // Receive sequence is minimum 480 us
}

uint8_t _temp_check(const temp_def *def)
{
  return GPIO_ReadInputDataBit(def->port, def->pin);
}

uint8_t _temp_read(const temp_def *def)
{
  uint8_t val = 0;

  _temp_pin_out(def);

  for (uint8_t i = 0; i < 8; i++)
  {
    _temp_pin_down(def);

    sleep_us(2); // Pull low for minimum 1 us

    val >>= 1;

    _temp_pin_up(def);
    _temp_pin_in(def);

    sleep_us(2); // Sample within 15 us

    if (_temp_check(def))
    {
      val |= 0x80;
    }

    sleep_us(60); // Cycle lasts at least 60 us

    _temp_pin_out(def);
  }

  return val;
}

void _temp_write(const temp_def *def, uint8_t data)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    _temp_pin_down(def);

    sleep_us(2); // Pull low for minimum 1 us

    if (data & 0x01)
    {
      _temp_pin_up(def);
    }
    else
    {
      _temp_pin_down(def);
    }

    data >>= 1;

    sleep_us(60); // Cycle lasts at least 60 us

    _temp_pin_up(def);
  }

  _temp_pin_up(def);
}

