/******************************************************************************
motor_driver.cpp
Implementation of the TB6612FNG motor channel driver. See motor_driver.h.

Part of BlindsESP32: ESP32 smart blinds controlled from Arduino Cloud.
Copyright (c) 2026 Mike Costa
Released under the MIT License (see LICENSE).
******************************************************************************/

#include "motor_driver.h"

Motor::Motor(int in1, int in2, int pwm, int offset, int standby)
  : _in1(in1), _in2(in2), _pwm(pwm), _offset(offset), _standby(standby)
{
  pinMode(_in1, OUTPUT);
  pinMode(_in2, OUTPUT);
  pinMode(_pwm, OUTPUT);
  pinMode(_standby, OUTPUT);
}

void Motor::drive(int direction)
{
  if (direction == 0) return;
  digitalWrite(_standby, HIGH);
  bool forward = (direction * _offset) > 0;
  digitalWrite(_in1, forward ? HIGH : LOW);
  digitalWrite(_in2, forward ? LOW : HIGH);
  digitalWrite(_pwm, HIGH);
}

void Motor::brake()
{
  digitalWrite(_in1, HIGH);
  digitalWrite(_in2, HIGH);
  digitalWrite(_pwm, LOW);
}

void Motor::standby()
{
  digitalWrite(_standby, LOW);
}
