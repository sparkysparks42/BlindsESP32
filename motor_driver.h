/******************************************************************************
motor_driver.h
Minimal driver for one channel of a TB6612FNG dual H-bridge.
Full speed only: the PWM/enable pin is used as a plain on/off output.

Part of BlindsESP32: ESP32 smart blinds controlled from Arduino Cloud.
Copyright (c) 2026 Mike Costa
Released under the MIT License (see LICENSE).
******************************************************************************/

#ifndef MOTOR_DRIVER_H
#define MOTOR_DRIVER_H

#include <Arduino.h>

class Motor
{
  public:
    // Empty motor, so globals can be declared before setup() and assigned later.
    Motor() {}

    // in1/in2: direction pins, pwm: enable pin (driven fully on/off),
    // offset: 1 or -1 to flip this motor's direction, standby: shared STBY pin.
    Motor(int in1, int in2, int pwm, int offset, int standby);

    // Run at full speed. direction > 0 = forward, direction < 0 = reverse
    // (flipped by the motor's offset). Takes the driver out of standby.
    // Passing 0 does nothing; use brake() to stop.
    void drive(int direction);

    // Stop the motor (both inputs high, enable low).
    void brake();

    // Put the driver chip into standby. The next drive() wakes it.
    void standby();

  private:
    int _in1 = -1, _in2 = -1, _pwm = -1, _offset = 1, _standby = -1;
};

#endif
